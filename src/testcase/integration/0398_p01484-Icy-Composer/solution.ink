// Translated from solution.cpp.

var EPS = 1e-9;

var PI = acos(-1.0);

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic)
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic)
{
  return cpp_expression("#include <stdio.h> #inclu");
}

var n: dynamic;

var m: dynamic;

var k: dynamic;

var input = cpp_array(500, 6);

var str = cpp_array(10000100);

var opened: dynamic;

func Open(pos: dynamic)
{
  var ret: dynamic;
  while (((input[5][pos] != cpp_char("\u{0}")) && (input[5][pos] != cpp_char(")"))))
  {
    if (isdigit(input[5][pos]))
    {
      var num = atoi((input[5] + pos));
      var v = 0;
      while (isdigit(input[5][pos]))
      {
        pos += 1;
        v += 1;
      }
      assert((v <= 7));
      assert((input[5][pos] == cpp_char("(")));
      pos += 1;
      var nret = Open(pos);
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
      var add: dynamic;
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

func main()
{
  while ((scanf("%d %d %d", (&n), (&m), (&k)) > 0))
  {
    opened.clear();
    scanf("%s", input[5]);
    {
      var pos = 0;
      var s = Open(pos);
      sprintf(str, "%s", s.c_str());
    }
    var ans = -1;
    var maxValue = -1;
    printf("%d %d\n", ans, maxValue);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
        add += nret;
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      scanf("%s", input[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
        REP(l, (r + 1));
        {
          var c = input[iter][(r + 1)];
          input[iter][(r + 1)] = 0;
          lv += (strstr(str, (input[iter] + l)) != null);
          input[iter][(r + 1)] = c;
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var lv = 0;
      if ((lv > maxValue))
      {
        ans = (iter + 1);
        maxValue = lv;
      }
    }
