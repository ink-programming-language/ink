// Translated from solution.cpp.

func SORT(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> ty");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func hex(n: dynamic)
{
  if ((n / 16))
  {
    hex((n / 16));
  }
  var las = (n % 16);
  if ((las < 10))
  {
    write(char((las + cpp_char("0"))));
  } else
  {
    write(char(((las - 10) + cpp_char("a"))));
  }
}

func solve(argument_0: dynamic)
{
  var ins = cpp_construct(9);
  REP(i, 9);
  read(ins[i]);
  var cs = cpp_construct(9, 0);
  REP(i, 9);
  {
    REP(k, ins[i].size());
    {
      var j = ((ins[i].size() - 1) - k);
      if (((cpp_char("0") <= ins[i][j]) && (ins[i][j] <= cpp_char("9"))))
      {
        ins[i][j] -= cpp_char("0");
      } else
      {
        ins[i][j] = ((ins[i][j] - cpp_char("a")) + 10);
      }
      REP(l, 4);
      if ((ins[i][j] & ((1 << l))))
      {
        cs[i] = ((cs[i] | ((1 << ((l + (k * 4)))))));
      }
    }
  }
  var answer = 0;
  var carry = 0;
  REP(i, 32);
  {
    var pari = 0;
    REP(j, 8);
    if ((cs[j] & ((1 << i))))
    {
      pari += 1;
    }
    if ((carry & ((1 << i))))
    {
      pari += 1;
    }
    if ((((((cs[8] >> i)) & 1)) != ((pari & 1))))
    {
      if ((carry & ((1 << i))))
      {
        pari -= 2;
      }
      pari = (8 - pari);
      answer += ((1 << i));
    }
    carry += ((((pari / 2)) << ((i + 1))));
  }
  return answer;
}

func __cpp_top_level_1()
{
}

func main(argument_0: dynamic)
{
  var n: dynamic;
  read(n);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    hex(solve());
    write("\n");
  }
