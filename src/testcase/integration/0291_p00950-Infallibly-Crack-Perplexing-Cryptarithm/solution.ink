// Translated from solution.cpp.

var used: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var idx: dynamic = cpp_uninitialized();

var valid: dynamic = cpp_uninitialized();

var ch: dynamic = [cpp_char("0"), cpp_char("1"), cpp_char("+"), cpp_char("-"), cpp_char("*"), cpp_char("("), cpp_char(")"), cpp_char("=")];

var ord: dynamic = cpp_array(8);

func check(a: dynamic) -> dynamic
{
  var par: dynamic = 0;
  for (var s: dynamic in a)
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

func getNum() -> dynamic
{
  var res: dynamic = 0;
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

func cal() -> dynamic
{
  var ch: dynamic = S[idx];
  var res: dynamic = 0;
  var sign: dynamic = 1;
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

func bnf() -> dynamic
{
  var res: dynamic = cal();
  while ((idx < cpp_cast(S.size())))
  {
    if ((valid == 0))
    {
      return -1;
    }
    var ch: dynamic = S[idx];
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

func mkS(a: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  for (var s: dynamic in a)
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

func calc(A: dynamic, B: dynamic) -> dynamic
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
  var ra: dynamic = bnf();
  idx = 0;
  S = B;
  var rb: dynamic = bnf();
  return ((ra == rb) && valid);
}

func calc(s: dynamic) -> dynamic
{
  s = mkS(s);
  {
    var i: dynamic = 0;
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

func dfs(num: dynamic, s: dynamic) -> dynamic
{
  if ((num == 8))
  {
    return calc(s);
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  var str: dynamic = cpp_uninitialized();
  read(str);
  if ((str.size() < 3))
  {
    write(0, "\n");
    exit(0);
  }
  var cnt: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
  var c: dynamic = 0;
  for (var p: dynamic in cnt)
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
