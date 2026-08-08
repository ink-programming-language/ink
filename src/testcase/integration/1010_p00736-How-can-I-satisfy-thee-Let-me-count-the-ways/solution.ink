// Translated from solution.cpp.

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func repl(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(ll i=(ll)(a);i<(ll)(b);i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func each(itr: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/");
}

var pb = cpp_expression("#include");

func all(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc+");
}

func mmax(x: dynamic, y: dynamic)
{
  return cpp_expression("#include");
}

func mmin(x: dynamic, y: dynamic)
{
  return cpp_expression("#include");
}

func maxch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <b");
}

func minch(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <b");
}

func uni(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using");
}

func exist(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var bcnt = cpp_expression("#include <bits/std");

var INF = cpp_expression("#include");

var n: dynamic;

var s = cpp_array(111);

var sav = cpp_array(111);

func f(l: dynamic, r: dynamic)
{
  if (((l + 1) == r))
  {
    return (s[l] - cpp_char("0"));
  }
  if ((s[l] == cpp_char("-")))
  {
    return (2 - f((l + 1), r));
  }
  if ((s[l] == cpp_char("(")))
  {
    var level = -1;
  }
  return -1;
}

func main()
{
  cin.sync_with_stdio(false);
  while (1)
  {
    read(s);
    if ((s[0] == cpp_char(".")))
    {
      break;
    }
    var res = 0;
    n = strlen(s);
    rep(p, 3);
    rep(q, 3);
    rep(r, 3);
    {
      memcpy(sav, s, cpp_sizeof((s)));
      if ((f(0, n) == 2))
      {
        res += 1;
      }
      memcpy(s, sav, cpp_sizeof((sav)));
    }
    write(res, "\n");
  }
  return 0;
}

func repl(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
      if ((s[i] == cpp_char("(")))
      {
        level += 1;
      } else if ((s[i] == cpp_char(")")))
      {
        level -= 1;
      }
      if (((level == 0) && (s[i] == cpp_char("+"))))
      {
        return max(f((l + 1), i), f((i + 1), (r - 1)));
      }
      if (((level == 0) && (s[i] == cpp_char("*"))))
      {
        return min(f((l + 1), i), f((i + 1), (r - 1)));
      }
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if ((s[i] == cpp_char("P")))
        {
          s[i] = (p + cpp_char("0"));
        }
        if ((s[i] == cpp_char("Q")))
        {
          s[i] = (q + cpp_char("0"));
        }
        if ((s[i] == cpp_char("R")))
        {
          s[i] = (r + cpp_char("0"));
        }
      }
