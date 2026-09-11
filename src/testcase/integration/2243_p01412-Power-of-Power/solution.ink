// Translated from solution.cpp.

func REP2(i: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (int)(m); i < (int)(n); i++)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <ios");
}

func ALL(S: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #i");
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

var memo: dynamic = cpp_array(2, 110, 110);

func rec(o: dynamic, z: dynamic, num: dynamic) -> dynamic
{
  if (((o == 0) && (z == 0)))
  {
    return  ((num == 1)) ? "" : "2";
  }
  if ((memo[o][z][num] != ""))
  {
    return memo[o][z][num];
  }
  var res: dynamic = cpp_assign(memo[o][z][num], "=", "2");
  if ((((num == 0) && (z > 0)) && (rec(o, (z - 1), 1) != "2")))
  {
    res = (cpp_char("0") + rec(o, (z - 1), 1));
  }
  if ((num == 1))
  {
    if (((o > 0) && (rec((o - 1), z, 0) != "2")))
    {
      res = min(res, (cpp_char("1") + rec((o - 1), z, 0)));
    }
    if (((o > 0) && (rec((o - 1), z, 1) != "2")))
    {
      res = min(res, (cpp_char("1") + rec((o - 1), z, 1)));
    }
    if (((z > 0) && (rec(o, (z - 1), 0) != "2")))
    {
      res = min(res, (cpp_char("0") + rec(o, (z - 1), 0)));
    }
  }
  return res;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  read(N);
  var B: dynamic = cpp_uninitialized();
  var O: dynamic = cpp_uninitialized();
  var Z: dynamic = cpp_uninitialized();
  sort(ALL(B));
  if (((((Z.size() % 2) != 0) && (O.size() == 0)) && (B.size() != 0)))
  {
    O.push_back(B[0]);
    reverse(ALL(B));
    B.pop_back();
    sort(ALL(B));
  }
  if ((((cpp_cast(B.size()) > 1) && (B[(B.size() - 2)] == 2)) && (B[(B.size() - 1)] == 3)))
  {
    swap(B[(B.size() - 2)], B[(B.size() - 1)]);
  }
  ((REP(i, B.size()) << B[i]) << endl);
  var str: dynamic = rec(O.size(), Z.size(), 1);
  if ((str == "2"))
  {
    reverse(ALL(O));
    ((REP(i, Z.size()) << Z[i]) << endl);
    ((REP(i, O.size()) << O[i]) << endl);
  } else
  {
    ((REP(i, str.size()) << ( ((str[i] == cpp_char("0"))) ? 0 : O[0])) << endl);
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var num: dynamic = cpp_uninitialized();
    read(num);
    if ((num == 0))
    {
      Z.push_back(num);
    } else if ((num == 1))
    {
      O.push_back(num);
    } else
    {
      B.push_back(num);
    }
  }
