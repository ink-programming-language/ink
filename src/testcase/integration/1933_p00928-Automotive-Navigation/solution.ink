// Translated from solution.cpp.

func for_cpp(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;++i)");
}

func for_rev(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i>=b;--i)");
}

func allof(a: dynamic)
{
  return cpp_expression("// tsukasa_diary'");
}

func minit(a: dynamic, b: dynamic)
{
  return cpp_expression("// tsukasa_diary's pr");
}

func size_of(a: dynamic)
{
  return cpp_expression("// tsukasa_di");
}

func POPCNT(x: dynamic)
{
  return builtin_popcount(x);
}

func POPCNT(x: dynamic)
{
  return builtin_popcountll(x);
}

var iINF = (1 << 30);

var lINF = (1 << 60);

var EPS = 1e-9;

func in_range(v: dynamic, mx: dynamic, mi: dynamic)
{
  return ((mi <= v) && (v < mx));
}

func in_range(v: dynamic, mi: dynamic, mx: dynamic)
{
  return (((-EPS) < (v - mi)) && ((v - mx) < EPS));
}

func in_range(x: dynamic, y: dynamic, W: dynamic, H: dynamic)
{
  return ((((0 <= x) && (x < W)) && (0 <= y)) && (y < H));
}

var DX = [0, 1, 0, -1];

var DY = [1, 0, -1, 0];

var DX = [0, 1, 1, 1, 0, -1, -1, -1];

var DY = [-1, -1, 0, 1, 1, 1, 0, -1];

var n: dynamic;

var stx: dynamic;

var sty: dynamic;

var t: dynamic;

var loads: dynamic;

var d = cpp_array(110);

var c = cpp_array(110);

var dp = cpp_array(4, 4, 55, 55, 1010);

func solve()
{
  minit(dp, 0);
  for_cpp(r, 0, 4);
  fill(dp[0][sty][stx][r], (dp[0][sty][stx][r] + 4), 1);
  var i = 0;
  for_cpp(ii, 0, t);
  {
    for_cpp(rep, 0, d[ii]);
    {
      for_cpp(y, 0, 55);
      for_cpp(x, 0, 55);
      for_cpp(r, 0, 4);
      {
        var flag = false;
        for_cpp(pr, 0, 4) |= dp[i][y][x][r][pr];
        if ((!flag))
        {
          continue;
        }
        var nx = (x + DX[r]);
        var ny = (y + DY[r]);
        if ((loads.find(ld) == loads.end()))
        {
          continue;
        }
        for_cpp(nr, 0, 4);
        {
          if ((nr == 2))
          {
            continue;
          }
          var dir = (((r + nr)) % 4);
          dp[(i + 1)][ny][nx][dir][r] = 1;
        }
      }
      i += 1;
    }
    var ok = 0;
    if ((c[ii] == cpp_char("S")))
    {
      ok = 2;
    }
    if ((c[ii] == cpp_char("E")))
    {
      ok = 1;
    }
    if ((c[ii] == cpp_char("W")))
    {
      ok = 3;
    }
    for_cpp(y, 0, 55);
    for_cpp(x, 0, 55);
    for_cpp(r, 0, 4);
    for_cpp(pr, 0, 4);
    {
      if (((r != ok) && (pr != ok)))
      {
        dp[i][y][x][r][pr] = 0;
      }
    }
  }
  for_cpp(x, 0, 55);
  for_cpp(y, 0, 55);
  {
    var flag = false;
    for_cpp(r, 0, 4);
    {
      var xx = (x + DX[r]);
      var yy = (y + DY[r]);
      if ((loads.find(ld) == loads.end()))
      {
        continue;
      }
      for_cpp(pr, 0, 4) |= dp[i][y][x][r][pr];
    }
    if (flag)
    {
      write(x, " ", y, "\n");
    }
  }
}

func main()
{
  read(n, stx, sty, t);
  for_cpp(i, 0, n);
  {
    var sx: dynamic;
    var sy: dynamic;
    var ex: dynamic;
    var ey: dynamic;
    read(sx, sy, ex, ey);
    var x = min(sx, ex);
    var xx = max(sx, ex);
    {
      var j = 0;
      while (((x + j) < xx))
      {
        loads.insert(Load(Point((x + j), sy), Point(((x + j) + 1), sy)));
        loads.insert(Load(Point(((x + j) + 1), sy), Point((x + j), sy)));
        j += 1;
      }
    }
    var y = min(sy, ey);
    var yy = max(sy, ey);
    {
      var j = 0;
      while (((y + j) < yy))
      {
        loads.insert(Load(Point(sx, (y + j)), Point(sx, ((y + j) + 1))));
        loads.insert(Load(Point(sx, ((y + j) + 1)), Point(sx, (y + j))));
        j += 1;
      }
    }
  }
  for_cpp(i, 0, t);
  read(d[i], c[i]);
  solve();
  return 0;
}
