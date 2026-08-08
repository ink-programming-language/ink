// Translated from solution.cpp.

var used: dynamic;

var M: dynamic;

var S: dynamic;

var idx: dynamic;

var valid: dynamic;

var ch = [cpp_char("0"), cpp_char("1"), cpp_char("+"), cpp_char("-"), cpp_char("*"), cpp_char("("), cpp_char(")"), cpp_char("=")];

var ord = cpp_array(8);

func check(a: dynamic)
{
  var par = 0;
  for (var s in a)
  {
    par += (((s == cpp_char("("))) - ((s == cpp_char(")"))));
    if ((s == cpp_char("=")))
    {
      return 0;
    }
    if ((par < 0))
    {
      return 0;
    }
  }
  return (par == 0);
}

func getNum()
{
  var res = 0;
  if (((S[idx] == cpp_char("0")) && isdigit(S[(idx + 1)])))
  {
    valid = 0;
  }
  while (isdigit(S[idx]))
  {
    res = (((res * 2) + S[cpp_update(idx, "++")]) - cpp_char("0"));
  }
  return res;
}

func cal()
{
  var ch = S[idx];
  var res = 0;
  var sign = 1;
  if ((((ch == cpp_char("+")) || (ch == cpp_char("*"))) || (ch == cpp_char(")"))))
  {
    valid = 0;
    return 0;
  }
  while ((S[idx] == cpp_char("-")))
  {
    idx += 1;
    sign *= -1;
  }
  ch = S[idx];
  if (isdigit(ch))
  {
    res = (sign * getNum());
    if ((S[idx] == cpp_char("*")))
    {
      idx += 1;
      return (res * cal());
    }
    return res;
  } else if ((ch == cpp_char("(")))
  {
    idx += 1;
    res = (sign * bnf());
    idx += 1;
    return res;
  }
  valid = 0;
  return 0;
}

func bnf()
{
  var res = cal();
  while ((idx < cpp_cast(S.size())))
  {
    if ((valid == 0))
    {
      return -1;
    }
    var ch = S[idx];
    if ((ch == cpp_char("(")))
    {
      valid = 0;
    } else if ((ch == cpp_char("*")))
    {
      idx += 1;
      res *= cal();
    } else if ((ch == cpp_char("+")))
    {
      idx += 1;
      res += cal();
    } else if ((ch == cpp_char("-")))
    {
      idx += 1;
      res -= cal();
    } else if ((ch != cpp_char(")")))
    {
      valid = 0;
    } else
    {
      break;
    }
  }
  if ((valid == 0))
  {
    return -1;
  }
  return res;
}

func mkS(a: dynamic)
{
  var res: dynamic;
  for (var s in a)
  {
    if (isalpha(s))
    {
      res += ch[ord[M[s]]];
    } else
    {
      res += s;
    }
  }
  return res;
}

func calc(A: dynamic, B: dynamic)
{
  if (((A.size() == 0) || (B.size() == 0)))
  {
    return 0;
  }
  if (used.count(P(A, B)))
  {
    return 0;
  }
  used.insert(P(A, B));
  valid = (check(A) && check(B));
  idx = 0;
  S = A;
  var ra = bnf();
  idx = 0;
  S = B;
  var rb = bnf();
  return ((ra == rb) && valid);
}

func calc(s: dynamic)
{
  s = mkS(s);
  {
    var i = 0;
    while ((i < cpp_cast(s.size())))
    {
      if ((s[i] == cpp_char("=")))
      {
        return calc(s.substr(0, i), s.substr((i + 1), ((s.size() - i) - 1)));
      }
      i += 1;
    }
  }
  return 0;
}

func dfs(num: dynamic, s: dynamic)
{
  if ((num == 8))
  {
    return calc(s);
  }
  var res = 0;
  {
    var i = 0;
    while ((i < 8))
    {
      if ((ord[i] != -1))
      {
        i += 1;
        continue;
      }
      ord[i] = num;
      res += dfs((num + 1), s);
      ord[i] = -1;
      i += 1;
    }
  }
  return res;
}

func main()
{
  var str: dynamic;
  read(str);
  if ((str.size() < 3))
  {
    write(0, "\n");
    exit(0);
  }
  var cnt: dynamic;
  {
    var i = 0;
    while ((i < cpp_cast(str.size())))
    {
      if (isalpha(str[i]))
      {
        cnt[str[i]] += 1;
      }
      i += 1;
    }
  }
  if ((cnt.size() > 8))
  {
    write(0, "\n");
    exit(0);
  }
  var c = 0;
  for (var p in cnt)
  {
    if (isalpha(p.first))
    {
      M[p.first] = cpp_update(c, "++");
    }
  }
  memset(ord, -1, cpp_sizeof((ord)));
  write(dfs(0, str), "\n");
  return 0;
}
