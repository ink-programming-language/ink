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

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var input: dynamic = cpp_array(500, 6);

var str: dynamic = cpp_array(10000100);

var opened: dynamic = cpp_uninitialized();

func Open(pos: dynamic) -> dynamic
{
  var ret: dynamic = cpp_uninitialized();
  while (((input[5][pos] != cpp_char("\u{0}")) && (input[5][pos] != cpp_char(")"))))
  {
    if (isdigit(input[5][pos]))
    {
      var num: dynamic = atoi((input[5] + pos));
      var v: dynamic = 0;
      while (isdigit(input[5][pos]))
      {
        pos += 1;
        v += 1;
      }
      assert((v <= 7));
      assert((input[5][pos] == cpp_char("(")));
      pos += 1;
      var nret: dynamic = Open(pos);
      assert((input[5][pos] == cpp_char(")")));
      pos += 1;
      num = min(num, max(2, ((((m + cpp_cast(nret.size())) - 1)) / cpp_cast(nret.size()))));
      if ((nret.size() >= 400))
      {
        if (opened.count(nret))
        {
          num = 1;
        }
        opened.insert(nret);
      }
      var add: dynamic = cpp_uninitialized();
      if ((add.size() >= 400))
      {
        opened.insert(add);
      }
      ret += add;
    } else
    {
      ret += input[5][cpp_update(pos, "++")];
    }
  }
  assert((cpp_cast(ret.size()) <= 10000000));
  return ret;
}

func main() -> dynamic
{
  while ((scanf("%d %d %d", (&n), (&m), (&k)) > 0))
  {
    opened.clear();
    scanf("%s", input[5]);
    {
      var pos: dynamic = 0;
      var s: dynamic = Open(pos);
      sprintf(str, "%s", s.c_str());
    }
    var ans: dynamic = -1;
    var maxValue: dynamic = -1;
    printf("%d %d\n", ans, maxValue);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        add += nret;
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      scanf("%s", input[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        REP(l, (r + 1));
        {
          var c: dynamic = input[iter][(r + 1)];
          input[iter][(r + 1)] = 0;
          lv += (strstr(str, (input[iter] + l)) != null);
          input[iter][(r + 1)] = c;
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var lv: dynamic = 0;
      if ((lv > maxValue))
      {
        ans = (iter + 1);
        maxValue = lv;
      }
    }
