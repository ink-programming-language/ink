// Translated from solution.cpp.

var EPS: dynamic = 1e-9;

var PI: dynamic = acos(-1.0);

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic) -> dynamic
{
  return cpp_expression("#include <stdio.h> #inclu");
}

func Calc(ys: dynamic, vys: dynamic, n: dynamic) -> dynamic
{
  REP(i, 210);
  {
    ys[0][i] = 1e+100;
    ys[1][i] = -1e+100;
  }
}

func CalcVolume(x: dynamic, py: dynamic, ny: dynamic, pz: dynamic, nz: dynamic) -> dynamic
{
  var f1: dynamic = (py * pz);
  var f2: dynamic = (((((py + ny)) / 2.0) * ((pz + nz))) / 2.0);
  var f3: dynamic = (ny * nz);
  var ret: dynamic = ((((f1 + (4 * f2)) + f3)) / 6.0);
  return ret;
}

var vys: dynamic = cpp_array(410);

var vzs: dynamic = cpp_array(410);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var ys: dynamic = cpp_array(210, 2);

var zs: dynamic = cpp_array(210, 2);

func main() -> dynamic
{
  while (cpp_comma(scanf("%d %d", (&m), (&n)), (n | m)))
  {
    MEMSET(vys, 0);
    MEMSET(vzs, 0);
    Calc(ys, vys, m);
    Calc(zs, vzs, n);
    var ans: dynamic = 0.0;
    {
      var x: dynamic = 0;
      while ((x <= 200))
      {
        var px: dynamic = x;
        var nx: dynamic = (x + 1);
        if (((((ys[0][px] > ys[1][px]) || (ys[0][nx] > ys[1][nx])) || (zs[0][px] > zs[1][px])) || (zs[0][nx] > zs[1][nx])))
        {
          x += 1;
          continue;
        }
        ans += CalcVolume((x - 100), (ys[1][px] - ys[0][px]), (ys[1][nx] - ys[0][nx]), (zs[1][px] - zs[0][px]), (zs[1][nx] - zs[0][nx]));
        x += 1;
      }
    }
    printf("%.8f\n", ans);
  }
}

func FOREQ(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic) -> dynamic
{
      var d1: dynamic = abs((x - px));
      var d2: dynamic = abs((nx - x));
      var y: dynamic = ((((py * d2) + (ny * d1))) / ((d1 + d2)));
      ys[0][(x + 100)] = min(ys[0][(x + 100)], y);
      ys[1][(x + 100)] = max(ys[1][(x + 100)], y);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var px: dynamic = vys[i].first;
    var nx: dynamic = vys[(i + 1)].first;
    var py: dynamic = vys[i].second;
    var ny: dynamic = vys[(i + 1)].second;
    if ((px > nx))
    {
      swap(px, nx);
      swap(py, ny);
    }
    if ((px == nx))
    {
      continue;
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d %d", (&x), (&y));
      vys[(i + m)] = cpp_assign(vys[i], "=", make_pair(x, y));
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      scanf("%d %d", (&x), (&z));
      vzs[(i + n)] = cpp_assign(vzs[i], "=", make_pair(x, z));
    }
