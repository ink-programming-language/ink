// Translated from solution.cpp.

var MAXN: dynamic = (int_cpp(2e5) + 10);

var MOD: dynamic = (int_cpp(1e9) + 7);

var oo: dynamic = INT_MAX;

class Query
{
  var d: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_array(10, MAXN);

var open: dynamic = cpp_array(MAXN);

var close: dynamic = cpp_array(MAXN);

var bit: dynamic = cpp_array(MAXN);

func update(i: dynamic) -> dynamic
{
  {
    i = (MAXN - i);
    while ((i < MAXN))
    {
      bit[i] += 1;
      i += (i & (-i));
    }
  }
}

func query(i: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  {
    i = (MAXN - i);
    while ((i > 0))
    {
      sum += bit[i];
      i -= (i & (-i));
    }
  }
  return sum;
}

func gauss(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    return 0;
  }
  return ((x * ((x - 1))) / 2);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  while (((cin >> n) >> q))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(p[i]);
        open[i].clear();
        close[i].clear();
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < q))
      {
        var l: dynamic = cpp_uninitialized();
        var d: dynamic = cpp_uninitialized();
        var r: dynamic = cpp_uninitialized();
        var u: dynamic = cpp_uninitialized();
        read(l, d, r, u);
        open[l].push_back([d, u, i]);
        close[r].push_back([d, u, i]);
        i += 1;
      }
    }
    memset(ans, 0, cpp_sizeof((ans)));
    memset(bit, 0, cpp_sizeof((bit)));
    {
      var i: dynamic = n;
      while ((i > 0))
      {
        for (var e: dynamic in close[i])
        {
          ans[e.id][3] = query((e.u + 1));
          ans[e.id][6] = (query(e.d) - ans[e.id][3]);
          ans[e.id][9] = ((query(1) - ans[e.id][6]) - ans[e.id][3]);
        }
        update(p[i]);
        for (var e: dynamic in open[i])
        {
          ans[e.id][2] = (query((e.u + 1)) - ans[e.id][3]);
          ans[e.id][5] = (((query(e.d) - ans[e.id][2]) - ans[e.id][3]) - ans[e.id][6]);
          ans[e.id][8] = (((((query(1) - ans[e.id][5]) - ans[e.id][6]) - ans[e.id][2]) - ans[e.id][3]) - ans[e.id][9]);
        }
        i -= 1;
      }
    }
    memset(bit, 0, cpp_sizeof((bit)));
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        for (var e: dynamic in open[i])
        {
          ans[e.id][1] = query((e.u + 1));
          ans[e.id][4] = (query(e.d) - ans[e.id][1]);
          ans[e.id][7] = ((query(1) - ans[e.id][4]) - ans[e.id][1]);
        }
        update(p[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < q))
      {
        var r: dynamic = 0;
        r += (ans[i][1] * (((ans[i][6] + ans[i][8]) + ans[i][9])));
        r += (ans[i][2] * (((((ans[i][4] + ans[i][6]) + ans[i][7]) + ans[i][8]) + ans[i][9])));
        r += (ans[i][3] * (((ans[i][4] + ans[i][7]) + ans[i][8])));
        r += (ans[i][4] * (((ans[i][6] + ans[i][8]) + ans[i][9])));
        r += (ans[i][6] * ((ans[i][7] + ans[i][8])));
        r += (ans[i][5] * ((n - ans[i][5])));
        r += gauss(ans[i][5]);
        write(r, cpp_char("\n"));
        i += 1;
      }
    }
  }
  return 0;
}
