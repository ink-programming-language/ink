// Translated from solution.cpp.

func for_cpp(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;++i)");
}

func for_rev(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i>=b;--i)");
}

func allof(a: dynamic) -> dynamic
{
  return cpp_expression("// tsukasa_diary'");
}

func minit(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("// tsukasa_diary's pr");
}

func size_of(a: dynamic) -> dynamic
{
  return cpp_expression("// tsukasa_di");
}

func POPCNT(x: dynamic) -> dynamic
{
  return builtin_popcount(x);
}

func POPCNT(x: dynamic) -> dynamic
{
  return builtin_popcountll(x);
}

var iINF: dynamic = (1 << 30);

var lINF: dynamic = (1 << 60);

var EPS: dynamic = 1e-9;

func in_range(v: dynamic, mx: dynamic, mi: dynamic) -> dynamic
{
  return ((mi <= v) && (v < mx));
}

func in_range(v: dynamic, mi: dynamic, mx: dynamic) -> dynamic
{
  return (((-EPS) < (v - mi)) && ((v - mx) < EPS));
}

func in_range(x: dynamic, y: dynamic, W: dynamic, H: dynamic) -> dynamic
{
  return ((((0 <= x) && (x < W)) && (0 <= y)) && (y < H));
}

var DX: dynamic = [0, 1, 0, -1];

var DY: dynamic = [1, 0, -1, 0];

var DX: dynamic = [0, 1, 1, 1, 0, -1, -1, -1];

var DY: dynamic = [-1, -1, 0, 1, 1, 1, 0, -1];

var n: dynamic = cpp_uninitialized();

var stx: dynamic = cpp_uninitialized();

var sty: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var loads: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(110);

var c: dynamic = cpp_array(110);

var dp: dynamic = cpp_array(4, 4, 55, 55, 1010);

func solve() -> dynamic
{
  minit(dp, 0);
  for_cpp(r, 0, 4);
  fill(dp[0][sty][stx][r], (dp[0][sty][stx][r] + 4), 1);
  var i: dynamic = 0;
  for_cpp(ii, 0, t);
  {
    for_cpp(rep, 0, d[ii]);
    {
      for_cpp(y, 0, 55);
      for_cpp(x, 0, 55);
      for_cpp(r, 0, 4);
      {
        var flag: dynamic = false;
        for_cpp(pr, 0, 4) |= dp[i][y][x][r][pr];
        if ((!flag))
        {
          continue;
        }
        var nx: dynamic = (x + DX[r]);
        var ny: dynamic = (y + DY[r]);
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
          var dir: dynamic = (((r + nr)) % 4);
          dp[(i + 1)][ny][nx][dir][r] = 1;
        }
      }
      i += 1;
    }
    var ok: dynamic = 0;
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
    var flag: dynamic = false;
    for_cpp(r, 0, 4);
    {
      var xx: dynamic = (x + DX[r]);
      var yy: dynamic = (y + DY[r]);
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

func main() -> dynamic
{
  read(n, stx, sty, t);
  for_cpp(i, 0, n);
  {
    var sx: dynamic = cpp_uninitialized();
    var sy: dynamic = cpp_uninitialized();
    var ex: dynamic = cpp_uninitialized();
    var ey: dynamic = cpp_uninitialized();
    read(sx, sy, ex, ey);
    var x: dynamic = min(sx, ex);
    var xx: dynamic = max(sx, ex);
    {
      var j: dynamic = 0;
      while (((x + j) < xx))
      {
        loads.insert(Load(Point((x + j), sy), Point(((x + j) + 1), sy)));
        loads.insert(Load(Point(((x + j) + 1), sy), Point((x + j), sy)));
        j += 1;
      }
    }
    var y: dynamic = min(sy, ey);
    var yy: dynamic = max(sy, ey);
    {
      var j: dynamic = 0;
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
