// Translated from solution.cpp.

func repl(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)(a);i<(int)(b);i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

var INF: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

class dice
{
  var a: dynamic = cpp_array(6);
}

func rot(a: dynamic, b: dynamic, c: dynamic, d: dynamic) -> dynamic
{
  var tmp: dynamic = a;
  a = b;
  b = c;
  c = d;
  d = tmp;
}

func roll_f(d: dynamic) -> dynamic
{
  rot(d.a[0], d.a[3], d.a[2], d.a[1]);
}

func roll_b(d: dynamic) -> dynamic
{
  rot(d.a[0], d.a[1], d.a[2], d.a[3]);
}

func roll_l(d: dynamic) -> dynamic
{
  rot(d.a[3], d.a[5], d.a[1], d.a[4]);
}

func roll_r(d: dynamic) -> dynamic
{
  rot(d.a[3], d.a[4], d.a[1], d.a[5]);
}

var n: dynamic = cpp_uninitialized();

var nd: dynamic = cpp_array(20);

var dp: dynamic = cpp_array((1 << 15));

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  while (1)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    rep(i, 20)[i].clear();
    rep(i, (1 << 15));
    {
      dp[i] = 0;
    }
    rep(S, ((1 << n)));
    {
      var used: dynamic = cpp_uninitialized();
    }
    write(dp[(((1 << n)) - 1)], "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var d: dynamic = cpp_uninitialized();
      var sx: dynamic = cpp_uninitialized();
      var sy: dynamic = cpp_uninitialized();
      var s: dynamic = cpp_uninitialized();
      read(sx, sy);
      read(d.a[4], d.a[5], d.a[0], d.a[2], d.a[1], d.a[3]);
      read(s);
      nd[i][P(sx, sy)] = d.a[1];
      rep(j, s.length());
      {
        if ((s[j] == cpp_char("F")))
        {
          roll_f(d);
          sy -= 1;
        }
        if ((s[j] == cpp_char("B")))
        {
          roll_b(d);
          sy += 1;
        }
        if ((s[j] == cpp_char("R")))
        {
          roll_r(d);
          sx += 1;
        }
        if ((s[j] == cpp_char("L")))
        {
          roll_l(d);
          sx -= 1;
        }
        nd[i][P(sx, sy)] = d.a[1];
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if ((((S & ((1 << i)))) == 0))
        {
          continue;
        }
        for (var it: dynamic in nd[i])
        {
          used[it.fi] = true;
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if ((((S & ((1 << i)))) != 0))
        {
          continue;
        }
        var add: dynamic = 0;
        for (var it: dynamic in nd[i])
        {
          if (used[it.fi])
          {
            continue;
          }
          add += it.se;
        }
        var T: dynamic = (S | ((1 << i)));
        dp[T] = max(dp[T], (dp[S] + add));
      }
