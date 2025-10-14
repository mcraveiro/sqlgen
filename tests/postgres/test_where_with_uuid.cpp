#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <boost/lexical_cast.hpp>
#include <boost/uuid.hpp>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>

template <>
struct sqlgen::parsing::Parser<boost::uuids::uuid> {
  using Type = boost::uuids::uuid;

  static Result<boost::uuids::uuid> read(
      const std::optional<std::string>& _str) {
    if (!_str) {
      return error("boost::uuids::uuid cannot be NULL.");
    }
    return boost::lexical_cast<boost::uuids::uuid>(*_str);
  }

  static std::optional<std::string> write(const boost::uuids::uuid& _u) {
    return boost::uuids::to_string(_u);
  }

  static dynamic::Type to_type() noexcept {
    return sqlgen::dynamic::types::Dynamic{"TEXT"};
  }
};

namespace test_where_with_uuid {

struct Person {
  sqlgen::PrimaryKey<boost::uuids::uuid> id =
      boost::uuids::uuid(boost::uuids::random_generator()());
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(postgres, test_where_with_uuid) {
  const auto people = std::vector<Person>(
      {Person{.first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.first_name = "Marge", .last_name = "Simpson", .age = 42},
       Person{.first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{.first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto homer =
      sqlgen::postgres::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(write(std::ref(people)))
          .and_then(sqlgen::read<Person> |
                    where("id"_c ==
                          boost::uuids::to_string(people.front().id.value())));

  // const std::string expected = rfl::json::write(people.front());

  // EXPECT_EQ(rfl::json::write(people2), expected);
}

}  // namespace test_where_with_uuid

#endif
