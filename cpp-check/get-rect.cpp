#include "structures.hpp"

#include <CGAL/convex_hull_2.h>

//vectors with non-negative X,Y components that are (exactly) unit length:
std::vector< K::Vector_2 > const & unit_vectors() {
	static std::vector< K::Vector_2 > ret;
	if (ret.empty()) {
		/* This supposedly will hit all triples, but it seems to produce a few gaps
		std::vector< std::tuple< CGAL::Gmpz, CGAL::Gmpz, CGAL::Gmpz > > triples;
		const uint32_t Count = 3*100000 + 1;
		triples.reserve(Count);
		triples.emplace_back(3,4,5);
		for (uint32_t i = 0; i < (Count-1)/3; ++i) {
			assert(i < triples.size());
			CGAL::Gmpz a = std::get< 0 >(triples[i]);
			CGAL::Gmpz b = std::get< 1 >(triples[i]);
			CGAL::Gmpz c = std::get< 2 >(triples[i]);
			assert(a*a + b*b == c*c);
			assert(a > 0 && b > 0);
			triples.emplace_back(
				1 * a + -2 * b + 2 * c,
				2 * a + -1 * b + 2 * c,
				2 * a + -2 * b + 3 * c
			);
			triples.emplace_back(
				1 * a +  2 * b + 2 * c,
				2 * a +  1 * b + 2 * c,
				2 * a +  2 * b + 3 * c
			);
			triples.emplace_back(
				-1 * a + 2 * b + 2 * c,
				-2 * a + 1 * b + 2 * c,
				-2 * a + 2 * b + 3 * c
			);
		}
		assert(triples.size() == Count);

		std::vector< K::Vector_2 > vecs;
		vecs.reserve(triples.size()+2);
		for (auto const &t : triples) {
			CGAL::Gmpz const &a = std::get< 0 >(t);
			CGAL::Gmpz const &b = std::get< 1 >(t);
			CGAL::Gmpz const &c = std::get< 2 >(t);
			assert(a*a + b*b == c*c);
			vecs.emplace_back(CGAL::Gmpq(a, c), CGAL::Gmpq(b, c));
			assert(vecs.back() * vecs.back() == 1);
		}
		vecs.emplace_back(1,0);
		vecs.emplace_back(0,1);
		assert(vecs.size() == triples.size()+2);
		*/
		//N.b. might work better with a better starting triple.
		//lazier version based on lots of rotating:
		std::vector< K::Vector_2 > vecs;
		vecs.emplace_back(0,1);
		vecs.emplace_back(1,0);
		vecs.reserve(1002);
		while (vecs.size() < 1000) {
			auto const &v = vecs.back();
			K::Vector_2 perp(-v.y(), v.x());
			K::Vector_2 next = (v * 4 + perp * 3) / 5;
			if (next.x() < 0 || next.y() < 0) {
				next = K::Vector_2(-next.y(), next.x());
			}
			if (next.x() < 0 || next.y() < 0) {
				next = K::Vector_2(-next.y(), next.x());
			}
			if (next.x() < 0 || next.y() < 0) {
				next = K::Vector_2(-next.y(), next.x());
			}
			assert(next.x() >= 0 && next.y() >= 0);
			vecs.emplace_back(next);
			assert(vecs.back() * vecs.back() == 1);
		}


		//sort in counterclockwise order:
		std::sort(vecs.begin(), vecs.end(), [](K::Vector_2 const &a, K::Vector_2 const &b){
			return a.x() > b.x();
		});
		ret.reserve(vecs.size());

		CGAL::Gmpq thresh(std::cos(1.0f / 360.0f * M_PI));
		uint32_t over = 0;
		for (auto const &v : vecs) {
			if (!ret.empty() && ret.back() * v < thresh) ++over;
			if (ret.size() >= 2 && ret[ret.size()-2] * v > thresh) {
				ret.back() = v;
			} else {
				ret.emplace_back(v);
			}
		}
		std::cout << "Had " << over << " gaps with over threshold angle, made " << ret.size() << " angles." << std::endl;
	}
	return ret;
}

int main(int argc, char **argv) {
	if (argc != 2 && argc != 3) {
		std::cout << "Usage:\n\t./get-rect <problem> [solution-to-write]\n" << std::endl;
		return 1;
	}

	std::unique_ptr< Problem > problem = Problem::read(argv[1]);
	if (!problem) {
		std::cerr << "Failed to read problem." << std::endl;
		return 1;
	}
	std::cout << "Read problem with " << problem->silhouette.size() << " silhouette polygons and " << problem->skeleton.size() << " skeleton edges." << std::endl;

	//Idea: find closest bounding rectangle of silhouette, then fold that rectangle.

	std::vector< K::Point_2 > points;
	for (auto const &poly : problem->silhouette) {
		points.insert(points.end(), poly.begin(), poly.end());
	}

	std::vector< K::Point_2 > hull;

	CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter( hull ));

	std::cout << "Hull has " << hull.size() << " points." << std::endl;

	//Given the way that the score is computed (and_area / or_area)
	//it might actually make sense in some cases to avoid covering a portion of the figure.
	//particularly, in cases where the amount of and_area [as a fraction of total] is decreased less than the amount of or_area [as a fraction of total]
	
	//lazy version: ignore all of that.

	std::vector< K::Vector_2 > x_dirs = unit_vectors();
	

	{ //add directions that can be made rational:
		uint32_t added = 0;
		uint32_t skipped = 0;
		for (uint32_t e = 0; e < hull.size(); ++e) {
			auto const &a = hull[e];
			auto const &b = hull[(e+1)%hull.size()];
			K::Vector_2 dir(b-a);
			CGAL::Gmpq len2 = dir * dir;
			mpz_t num,den;
			if (mpz_root(num, len2.numerator().mpz(), 2) != 0 && mpz_root(den, len2.denominator().mpz(), 2)) {
				x_dirs.emplace_back((dir / CGAL::Gmpz(num)) * CGAL::Gmpz(den));
				++added;
			} else {
				++skipped;
			}
		}
		std::cout << "Addded " << added << " edge directions to test directions, skipped " << skipped << "." << std::endl;
	}


	auto get_score = [&problem](K::Vector_2 const &x, K::Point_2 const &min, K::Point_2 const &max) -> CGAL::Gmpq {
		CGAL::Polygon_set_2< K > a = problem->get_silhouette();
		CGAL::Polygon_2< K > b;

		{ //build square from min/max:
			K::Vector_2 y(-x.y(), x.x());
			b.push_back(K::Point_2(0,0) + min.x() * x + min.y() * y);
			b.push_back(K::Point_2(0,0) + max.x() * x + min.y() * y);
			b.push_back(K::Point_2(0,0) + max.x() * x + max.y() * y);
			b.push_back(K::Point_2(0,0) + min.x() * x + max.y() * y);
			assert(b.orientation() == CGAL::COUNTERCLOCKWISE);
		}

		CGAL::Polygon_set_2< K > a_and_b;
		CGAL::Polygon_set_2< K > a_or_b;
		a_and_b.join(a);
		a_and_b.intersection(b);
		a_or_b.join(a);
		a_or_b.join(b);

		auto polygon_with_holes_area = [](CGAL::Polygon_with_holes_2< K > const &pwh) -> CGAL::Gmpq {
			assert(!pwh.is_unbounded());
			auto ret = pwh.outer_boundary().area();
			assert(ret >= 0);
			for (auto hi = pwh.holes_begin(); hi != pwh.holes_end(); ++hi) {
				auto ha = hi->area();
				assert(ha <= 0);
				ret += ha;
			}
			assert(ret >= 0);
			return ret;
		};

		auto polygon_set_area = [&polygon_with_holes_area](CGAL::Polygon_set_2< K > const &ps) -> CGAL::Gmpq {
			std::list< CGAL::Polygon_with_holes_2< K > > res;
			ps.polygons_with_holes( std::back_inserter( res ) );
			CGAL::Gmpq ret = 0;
			for (auto const &poly : res) {
				ret += polygon_with_holes_area(poly);
			}
			assert(ret >= 0);
			return ret;
		};

		auto and_area = polygon_set_area(a_and_b);
		auto or_area = polygon_set_area(a_or_b);

		return and_area / or_area;
	};

	CGAL::Gmpq best_score = 0;
	struct {
		K::Vector_2 x = K::Vector_2(0,0);
		K::Point_2 min = K::Point_2(0,0); //relative to x, perp(x)
		K::Point_2 max = K::Point_2(0,0); //relative to x, perp(x)
	} best;
	for (auto const &x_dir : x_dirs) {
		assert(x_dir * x_dir == 1);
		K::Vector_2 y_dir(-x_dir.y(), x_dir.x());

		CGAL::Gmpq min_x = x_dir * K::Vector_2(hull[0].x(), hull[0].y());
		CGAL::Gmpq max_x = min_x;
		CGAL::Gmpq min_y = y_dir * K::Vector_2(hull[0].x(), hull[0].y());
		CGAL::Gmpq max_y = min_y;
		for (auto const &pt : hull) {
			CGAL::Gmpq x = x_dir * K::Vector_2(pt.x(), pt.y());
			CGAL::Gmpq y = y_dir * K::Vector_2(pt.x(), pt.y());
			if (x < min_x) min_x = x;
			if (x > max_x) max_x = x;
			if (y < min_y) min_y = y;
			if (y > max_y) max_y = y;
		}
		if (max_x > min_x + 1) {
			auto c = (max_x + min_x) / 2;
			min_x = c - CGAL::Gmpq(1,2);
			max_x = c + CGAL::Gmpq(1,2);
		}
		if (max_y > min_y + 1) {
			auto c = (max_y + min_y) / 2;
			min_y = c - CGAL::Gmpq(1,2);
			max_y = c + CGAL::Gmpq(1,2);
		}
		CGAL::Gmpq score = get_score(x_dir, K::Point_2(min_x, min_y), K::Point_2(max_x, max_y));
		if (score > best_score) {
			best_score = score;
			best.x = x_dir;
			best.min = K::Point_2(min_x, min_y);
			best.max = K::Point_2(max_x, max_y);
		}
	}

	std::cout << "Best score was " << best_score << " at " << best.x << std::endl;
	std::cout << "  box: " << best.min << " to " << best.max << std::endl;

	//zig-zag fold into proper-sized box:

	auto make_folds_dests = [](CGAL::Gmpq const &min, CGAL::Gmpq const &max, std::vector< CGAL::Gmpq > &folds, std::vector< CGAL::Gmpq > &dests) {
		assert(max > min);
		folds.push_back(0);
		dests.push_back(min);
		while (folds.back() < 1) {
			auto step = max - min;
			if (folds.back() + step > 1) step = 1 - folds.back();
			if (folds.size() % 2 == 1) {
				dests.push_back(dests.back() + step);
			} else {
				dests.push_back(dests.back() - step);
			}
			assert(dests.back() >= min);
			assert(dests.back() <= max);
			folds.push_back(folds.back() + step);
		}
		assert(folds.back() == 1);
		assert(dests.size() == folds.size());
		assert(folds.size() >= 2);
	};
	std::vector< CGAL::Gmpq > x_folds;
	std::vector< CGAL::Gmpq > x_dests;
	std::vector< CGAL::Gmpq > y_folds;
	std::vector< CGAL::Gmpq > y_dests;

	make_folds_dests(best.min.x(), best.max.x(), x_folds, x_dests);
	make_folds_dests(best.min.y(), best.max.y(), y_folds, y_dests);

	std::ostringstream out;

	out << x_folds.size() * y_folds.size() << std::endl; //number of vertices
	for (uint32_t iy = 0; iy < y_folds.size(); ++iy) {
		for (uint32_t ix = 0; ix < x_folds.size(); ++ix) {
			out << x_folds[ix] << "," << y_folds[iy] << std::endl;
		}
	}
	out << (x_folds.size()-1) * (y_folds.size()-1) << std::endl; //number of facets
	for (uint32_t iy = 0; iy + 1 < y_folds.size(); ++iy) {
		for (uint32_t ix = 0; ix + 1 < x_folds.size(); ++ix) {
			out << "4"
				<< " " << (ix + iy * x_folds.size())
				<< " " << (ix + 1 + iy * x_folds.size())
				<< " " << (ix + 1 + (iy + 1) * x_folds.size())
				<< " " << (ix + (iy + 1) * x_folds.size())
				<< std::endl;
		}
	}
	//destinations:
	for (uint32_t iy = 0; iy < y_folds.size(); ++iy) {
		for (uint32_t ix = 0; ix < x_folds.size(); ++ix) {
			auto pt = x_dests[ix] * best.x + y_dests[iy] * K::Vector_2(-best.x.y(), best.x.x());
			out << pt.x() << "," << pt.y() << std::endl;
		}
	}

	std::cout << " --- solution (score = " << best_score << ") --- \n" << out.str();

	if (argc == 3) {
		std::cout << "(Writing to " << argv[2] << ")" << std::endl;
		std::ofstream file(argv[2]);
		file << out.str();
	}


	return 0;
}
