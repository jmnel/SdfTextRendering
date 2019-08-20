#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <Text/OutlineExtractor.hpp>

namespace arc {

    namespace detail {

        enum class PathType { Line,
                              Conic,
                              Cubic };

        class Path {
        public:
            vector<Vec2d> points;

            Path( Vec2d p0, Vec2d p1 ) {
                points.emplace_back( p0 );
                points.emplace_back( p1 );
            }

            Path( Vec2d p0, Vec2d p1, Vec2d p2 ) {
                points.emplace_back( p0 );
                points.emplace_back( p1 );
                points.emplace_back( p2 );
            }

            Path( Vec2d p0, Vec2d p1, Vec2d p2, Vec2d p3 ) {
                points.emplace_back( p0 );
                points.emplace_back( p1 );
                points.emplace_back( p2 );
                points.emplace_back( p3 );
            }

            PathType getType() const {
                switch( points.size() ) {
                    case 2:
                        return PathType::Line;
                    case 3:
                        return PathType::Conic;
                    case 4:
                        return PathType::Cubic;
                    default:
                        assert( false );
                        return PathType::Line;
                }
            }
        };

        class Outline {
        public:
            vector<Path> paths;
            Vec2d start;
            bool reversed = false;
            Outline( Vec2d start ) {
                this->start = start;
            }

            void addPath( Path const &path ) {
                paths.push_back( path );
            }
        };

        class GlyphDecompContext {
        public:
            Vec2d position;
            vector<Outline> outlines;
            bool startOutline = false;
            virtual ~GlyphDecompContext() = default;
        };

        Vec2d ftToArc( FT_Vector const &vec ) {
            return {(double)vec.x / 64.0, (double)vec.y / 64.0};
        }

        static int ftMoveTo( const FT_Vector *to, void *user ) {
            GlyphDecompContext *context =
                reinterpret_cast<GlyphDecompContext *>( user );
            context->position = ftToArc( *to );
            context->outlines.push_back( Outline( ftToArc( *to ) ) );
            return 0;
        }

        static int ftLineTo( const FT_Vector *to, void *user ) {
            GlyphDecompContext *context =
                reinterpret_cast<GlyphDecompContext *>( user );
            assert( !context->outlines.empty() );
            Vec2d p0 = context->position;
            Vec2d p1 = ftToArc( *to );
            context->outlines.back().addPath( Path( p0, p1 ) );
            context->position = p1;
            return 0;
        }

        static int ftConicTo( const FT_Vector *control,
                              const FT_Vector *to,
                              void *user ) {
            GlyphDecompContext *context =
                reinterpret_cast<GlyphDecompContext *>( user );
            Vec2d p0 = context->position;
            Vec2d p1 = ftToArc( *control );
            Vec2d p2 = ftToArc( *to );
            context->outlines.back().addPath( Path( p0, p1, p2 ) );
            context->position = p2;
            return 0;
        }

        static int ftCubicTo( const FT_Vector *control1,
                              const FT_Vector *control2,
                              const FT_Vector *to,
                              void *user ) {
            GlyphDecompContext *context =
                reinterpret_cast<GlyphDecompContext *>( user );
            assert( !context->outlines.empty() );
            Vec2d p0 = context->position;
            Vec2d p1 = ftToArc( *control1 );
            Vec2d p2 = ftToArc( *control2 );
            Vec2d p3 = ftToArc( *to );
            context->outlines.back().addPath( Path( p0, p1, p2, p3 ) );
            context->position = p3;
            return 0;
        }
    }  // namespace detail

    using detail::ftConicTo;
    using detail::ftCubicTo;
    using detail::ftLineTo;
    using detail::ftMoveTo;
    using detail::ftToArc;
    using detail::GlyphDecompContext;
    using detail::Outline;
    using detail::Path;
    using detail::PathType;

    // -- decomposeGlyph function --
    bool decomposeGlyph( FT_GlyphSlot &glyphSlot, vector<Outline> &outlines ) {
        FT_Outline_Funcs decompFunctions;
        decompFunctions.move_to = &ftMoveTo;
        decompFunctions.line_to = &ftLineTo;
        decompFunctions.conic_to = &ftConicTo;
        decompFunctions.cubic_to = &ftCubicTo;
        decompFunctions.shift = 0;
        decompFunctions.delta = 0;
        GlyphDecompContext context = {};
        auto error = FT_Outline_Decompose(
            &glyphSlot->outline, &decompFunctions, &context );
        assert( error == FT_Err_Ok );
        outlines.clear();
        for( size_t i = 0; i < context.outlines.size(); i++ ) {
            outlines.push_back( context.outlines[i] );
        }
        return !outlines.empty();
        //assert(!outlines.empty());
        //return true;
    }

    // -- computeGlyphSDF function --
    PixelGrid<uint8_t> *OutlineExtractor::computeGlyphSDF( FontFace const &fontFace,
                                                           uint16_t gid,
                                                           float &correction,
                                                           double scale,
                                                           int glyphPadding ) {

        auto const &ftFace = fontFace.m_ftFace;

        auto error = FT_Load_Glyph( ftFace, gid, FT_LOAD_NO_SCALE );
        assert( error == FT_Err_Ok );

        FT_Glyph ftGlyph;
        error = FT_Get_Glyph( ftFace->glyph, &ftGlyph );
        assert( error == FT_Err_Ok );

        //        if( correction ) {
        //            *correction = (float)( ftFace->glyph->metrics.height -
        //                                   ftFace->glyph->metrics.horiBearingY ) /
        //                          64.0f;
        //        }

        correction = (float)( ftFace->glyph->metrics.height -
                              ftFace->glyph->metrics.horiBearingY ) /
                     64.0f;

        FT_BBox bBox;
        FT_Glyph_Get_CBox( ftGlyph, FT_GLYPH_BBOX_UNSCALED, &bBox );

        vector<Outline> outlines;
        auto hasOutlines = decomposeGlyph( ftFace->glyph, outlines );

        if( !hasOutlines ) {
            return nullptr;
        }

        // combine paths from glyph's outlines
        vector<Path *> paths;
        for( size_t iOutline = 0; iOutline < outlines.size(); iOutline++ ) {
            auto outline = &outlines[iOutline];
            for( size_t iPath = 0; iPath < outline->paths.size(); iPath++ ) {
                paths.push_back( &outline->paths[iPath] );
            }
        }

        // collect endpoitns for glyph's outline paths
        vector<Vec2d> endpoints;
        for( size_t iPath = 0; iPath < paths.size(); iPath++ ) {
            Vec2d p0( 0.0 );
            Vec2d p1( 0.0 );
            auto path = paths[iPath];
            if( path->getType() == PathType::Line ) {
                p0 = path->points[0];
                p1 = path->points[1];
            } else if( path->getType() == PathType::Conic ) {
                p0 = path->points[0];
                p1 = path->points[2];
            } else {
                assert( false );
            }
            endpoints.push_back( p1 );
        }

        int xMin = int( (double)( bBox.xMin / 64.0 ) * scale );
        int yMin = int( (double)( bBox.yMin / 64.0 ) * scale );
        int xMax = int( (double)( bBox.xMax / 64.0 ) * scale );
        int yMax = int( (double)( bBox.yMax / 64.0 ) * scale );

        int glyphWidth = xMax - xMin + 2 * glyphPadding;
        int glyphHeight = yMax - yMin + 2 * glyphPadding;

        assert( glyphWidth > 0 );
        assert( glyphHeight > 0 );

        PixelGrid<double> glyphSDF( glyphWidth, glyphHeight );
        glyphSDF.clear( 1.0 );

        // compute each pixel's sign
        for( int yTile = 0; yTile - glyphHeight; yTile++ ) {
            int y = glyphHeight - yTile - (int)glyphPadding + (int)yMin + 1;
            int xLower = xMin;
            int xUpper = xMax;

            double yReal = (double)y / scale;
            double xMinReal = (double)( xLower - glyphPadding ) / scale;
            double xMaxReal = (double)( xUpper + glyphPadding ) / scale;

            double fudgeFactor = 0.000001;
            Vec2d scan0( xMinReal, yReal + fudgeFactor );
            Vec2d scan1( xMaxReal, yReal + fudgeFactor );

            vector<Vec2d> hits;
            vector<int> flags;
            bool res = false;

            for( size_t iPath = 0; iPath < paths.size(); iPath++ ) {
                Path *path = paths[iPath];

                // case 1 : linear path
                if( path->getType() == PathType::Line ) {
                    Line<double> pathLine( path->points[0], path->points[1] );
                    Line<double> scanLine( scan0, scan1 );
                    Vec2d intersection( 0.0 );
                    res = intersectLineLine( pathLine, scanLine, intersection );

                    if( res ) {
                        hits.push_back( intersection );
                        int xPixel = int( intersection.x * scale + 0.5 );
                        flags.push_back( xPixel );
                    }
                }
                // case 2 : quadratic path
                else if( path->getType() == PathType::Conic ) {
                    vector<Vec2d> intersections;
                    res = intersectLineConicBezier(
                        scan0, scan1, path->points, intersections );
                    if( res ) {
                        assert( intersections.size() == 1 ||
                                intersections.size() == 2 );
                        for( size_t i = 0; i < intersections.size(); i++ ) {
                            int xPixel = int( intersections[i].x * scale + 0.5 );
                            flags.push_back( xPixel );
                        }
                    }
                }
                // case 3 : cubic path - not implemented
                else {
                    assert( false );
                }

                // check for endpoint proxmimity intersections
                if( !res ) {
                    Vec2d p = path->points[0];
                    Vec2d norm = normalize( scan1 - scan0 );
                    double comp = dot( p - scan0, norm );
                    Vec2d proj = scan0 + comp * norm;
                    double dist = lengthSquared( proj - p );
                    if( dist == 0.0 ) {
                        int xPixel = int( p.x * scale + 0.5 );
                        flags.push_back( xPixel );
                    }
                }
            }

            // add padding to all flags
            for( size_t iFlag = 0; iFlag < flags.size(); iFlag++ ) {
                flags[iFlag] += glyphPadding - xMin - 1;
            }
            // add left / right bounds and sort ascending
            flags.insert( flags.begin(), 0 );
            flags.push_back( (int)glyphWidth - 1 );
            //! @todo Replace with container sort
            std::sort( flags.begin(), flags.end() );

            for( size_t i = 0; i < flags.size() - 1; i++ ) {
                // when i is odd, where are inside boundary
                bool inside = i % 2 == 1;
                assert( flags[i] > -1 && flags[i] < glyphWidth );
                // case 2 : span is inside boundary
                if( inside ) {
                    int start = flags[i];
                    int end = flags[i + 1];
                    for( int x = start; x <= end; x++ ) {
                        glyphSDF.setAt( x, yTile, -1.0 );
                    }
                }
            }
        }

        // compute edge distance for each pixel
        for( int y = 0; y < glyphHeight; y++ ) {
            for( int x = 0; x < glyphWidth; x++ ) {
                double min = FLT_MAX;
                Vec2d cur = {
                    (double)( (int)x - (int)glyphPadding + 1 + (int)xMin ) /
                        scale,
                    (double)( (int)glyphSDF.getHeight() - (int)y -
                              (int)glyphPadding + 1 + (int)yMin ) /
                        scale};

                for( size_t iPath = 0; iPath < paths.size(); iPath++ ) {
                    auto path = paths[iPath];

                    // linear path
                    if( path->getType() == PathType::Line ) {
                        auto p0 = path->points[0];
                        auto p1 = path->points[1];
                        auto dir = normalize( p1 - p0 );

                        double comp = dot( cur - p0, dir );
                        auto proj = p0 + dir * comp;
                        double dist = length( proj - cur );

                        if( comp < 0.0 ) {
                            dist = length( p0 - cur );
                        } else if( comp > length( p1 - p0 ) ) {
                            dist = length( p1 - cur );
                        } else {
                            dist = length( proj - cur );
                        }
                        if( std::abs( dist ) < min ) {
                            min = dist;
                            assert( dist < 100.0 );
                        }
                    }
                    // quadratic bezier path
                    if( path->getType() == PathType::Conic ) {
                        double t = 0.0;
                        Vec2d proj = nearestPointOnBezier( cur, path->points, t );
                        double dist = length( cur - proj );
                        if( abs( dist ) < min ) {
                            min = dist;
                        }
                    }
                }

                assert( x > -1 && y > -1 );
                assert( x < glyphWidth && y < glyphHeight );

                double sdfSign = glyphSDF.getAt( x, y );
                double sdfCombined = sdfSign * min;

                assert( min >= 0.0 );

                glyphSDF.setAt( x, y, sdfCombined );
            }
        }

        glyphSDF.multiply( 0.5 );
        //glyphSDF.normalize();
        double clamp = 1.0;
        glyphSDF.clamp( -clamp, clamp );

        uint nDown = 2;
        size_t w = glyphWidth;
        size_t h = glyphHeight;
        for( size_t iDown = 0; iDown < nDown; iDown++ ) {
            w = w / 2;
            h = h / 2;
            for( size_t y = 0; y < h; y++ ) {
                for( size_t x = 0; x < w; x++ ) {
                    auto v00 = glyphSDF.getAt( x * 2, y * 2 );
                    auto v01 = glyphSDF.getAt( x * 2 + 1, y * 2 );
                    auto v10 = glyphSDF.getAt( x * 2 + 1, y * 2 );
                    auto v11 = glyphSDF.getAt( x * 2 + 1, y * 2 + 1 );
                    auto mu = 0.25 * ( v00 + v01 + v10 + v11 );
                    glyphSDF.setAt( x, y, mu );
                }
            }
        }
        auto pixels = new PixelGrid<uint8_t>( w, h );
        //
        for( size_t y = 0; y < h; y++ ) {
            for( size_t x = 0; x < w; x++ ) {
                // uint8_t r = glyphSDF.getAt(x, y) >= 0.0 ? 255 : 0;
                double v = ( -glyphSDF.getAt( x, y ) + 1.0 ) * 0.5;
                uint8_t a = uint8_t( v * 255.0 );
                pixels->setAt( x, h - y - 1, a );
            }
        }

        // auto img = png::image<png::rgba_pixel>(w, h);
        // for (int y = 0; y < h; y++) {
        // for (int x = 0; x < w; x++) {
        // auto v = pixels->getAt(x, y);
        // img[y][x] = png::rgba_pixel(v, 0, 0, 255);
        //}
        //}
        // img.write("/home/jacques/repos/ArcturusGame/build/glypht.png");
        return pixels;
    }
}  // namespace arc
