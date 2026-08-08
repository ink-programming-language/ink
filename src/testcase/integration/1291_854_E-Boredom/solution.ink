// Translated from solution.cpp.

var MAXN = (int_cpp(2e5) + 10);

var MOD = (int_cpp(1e9) + 7);

var oo = INT_MAX;

class Query
{
  var d: dynamic;
  var u: dynamic;
  var id: dynamic;
}

var n: dynamic;

var q: dynamic;

var p = cpp_array(MAXN);

var ans = cpp_array(10, MAXN);

var open = cpp_array(MAXN);

var close = cpp_array(MAXN);

var bit = cpp_array(MAXN);

func update(i: dynamic)
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

func query(i: dynamic)
{
  var sum = 0;
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

func gauss(x: dynamic)
{
  if ((x == 0))
  {
    return 0;
  }
  return ((x * ((x - 1))) / 2);
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  while (((cin >> n) >> q))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        read(p[i]);
        open[i].clear();
        close[i].clear();
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < q))
      {
        var l: dynamic;
        var d: dynamic;
        var r: dynamic;
        var u: dynamic;
        read(l, d, r, u);
        open[l].push_back([d, u, i]);
        close[r].push_back([d, u, i]);
        i += 1;
      }
    }
    memset(ans, 0, cpp_sizeof((ans)));
    memset(bit, 0, cpp_sizeof((bit)));
    {
      var i = n;
      while ((i > 0))
      {
        for (var e in close[i])
        {
          ans[e.id][3] = query((e.u + 1));
          ans[e.id][6] = (query(e.d) - ans[e.id][3]);
          ans[e.id][9] = ((query(1) - ans[e.id][6]) - ans[e.id][3]);
        }
        update(p[i]);
        for (var e in open[i])
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
      var i = 1;
      while ((i <= n))
      {
        for (var e in open[i])
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
      var i = 0;
      while ((i < q))
      {
        var r = 0;
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
