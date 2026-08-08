// Translated from solution.cpp.

var inf_int = (1e9 + 100);

var inf_ll = 8e18;

var pb = cpp_expression("// // Cre");

var eb = cpp_expression("// // Create");

var pi = 3.1415926535898;

var fi = cpp_expression("// //");

var se = cpp_expression("// //");

var sp = cpp_expression("// // Create");

func sz(a: dynamic)
{
  return cpp_expression("// // Created b");
}

func all(a: dynamic)
{
  return cpp_expression("// // Created by");
}

func debug()
{
  return cpp_expression("// // Created by Ильдар Ялалов on 14.01.2020. /");
}

func debug()
{
  return cpp_expression("//");
}

func debug_arr()
{
  return cpp_expression("//");
}

var MAXN = ((2e5 + 100));

var LOG = 21;

var mod = 998244353;

var a = cpp_array(MAXN);

func solve()
{
  var n: dynamic;
  read(n);
  var sum = [0, 0];
  {
    var i = 1;
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
      var i = 1;
      while ((i <= n))
      {
        write((if (((i & 1))) a[i] else 1), " ");
        i += 1;
      }
    }
  } else
  {
    {
      var i = 1;
      while ((i <= n))
      {
        write((if (((i & 1))) 1 else a[i]), " ");
        i += 1;
      }
    }
  }
  write("\n");
}

func main()
{
  freopen("../output.txt", "r", stdin);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  cout.setf(ios.fixed);
  cout.precision(15);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  debug(((1.0 * clock()) / CLOCKS_PER_SEC));
}
