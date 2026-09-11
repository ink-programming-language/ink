// Translated from solution.cpp.

var inf_int: dynamic = (1e9 + 100);

var inf_ll: dynamic = 8e18;

var pb: dynamic = cpp_expression("// // Cre");

var eb: dynamic = cpp_expression("// // Create");

var pi: dynamic = 3.1415926535898;

var fi: dynamic = cpp_expression("// //");

var se: dynamic = cpp_expression("// //");

var sp: dynamic = cpp_expression("// // Create");

func sz(a: dynamic) -> dynamic
{
  return cpp_expression("// // Created b");
}

func all(a: dynamic) -> dynamic
{
  return cpp_expression("// // Created by");
}

func debug() -> dynamic
{
  return cpp_expression("// // Created by Ильдар Ялалов on 14.01.2020. /");
}

func debug() -> dynamic
{
  return cpp_expression("//");
}

func debug_arr() -> dynamic
{
  return cpp_expression("//");
}

var MAXN: dynamic = ((2e5 + 100));

var LOG: dynamic = 21;

var mod: dynamic = 998244353;

var a: dynamic = cpp_array(MAXN);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var sum: dynamic = [0, 0];
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      sum[(i & 1)] += a[i];
      i += 1;
    }
  }
  if ((sum[1] > sum[0]))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        write(( (((i & 1))) ? a[i] : 1), " ");
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        write(( (((i & 1))) ? 1 : a[i]), " ");
        i += 1;
      }
    }
  }
  write("\n");
}

func main() -> dynamic
{
  freopen("../output.txt", "r", stdin);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(15);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  debug(((1.0 * clock()) / CLOCKS_PER_SEC));
}
