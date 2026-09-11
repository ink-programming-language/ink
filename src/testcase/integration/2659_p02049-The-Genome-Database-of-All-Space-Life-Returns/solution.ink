// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func __cpp_top_level_1() -> dynamic
{
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func __cpp_top_level_2() -> dynamic
{
}

class RollingHash
{
  var hash: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  func RollingHash() -> dynamic
  {
    }
  func RollingHash(s: dynamic) -> dynamic
  {
      var n: dynamic = s.size();
      hash.assign((n + 1), 0);
      p.assign((n + 1), 1);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          hash[(i + 1)] = ((((hash[i] * B) + s[i])) % MOD);
          p[(i + 1)] = ((p[i] * B) % MOD);
          i += 1;
        }
      }
    }
  func find(l: dynamic, r: dynamic) -> dynamic
  {
      var res: dynamic = ((hash[r] + MOD) - ((hash[l] * p[(r - l)]) % MOD));
      return  ((res >= MOD)) ? (res - MOD) : res;
    }
}

var MOD: dynamic = (1e9 + 7);

var B: dynamic = 1777771;

var pat: dynamic = cpp_uninitialized();

var base: dynamic = cpp_uninitialized();

func calc(s: dynamic, t: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var n: dynamic = s.size();
  var m: dynamic = t.size();
  var len: dynamic = pat.size();
  if (((n + m) < len))
  {
    return 0;
  }
  var rh: dynamic = cpp_construct((s + t));
  {
    var i: dynamic = 0;
    while (((i < n) && ((i + len) <= (n + m))))
    {
      if (((i + len) <= n))
      {
        i += 1;
        continue;
      }
      if ((rh.find(i, (i + len)) == base.find(0, len)))
      {
        res += 1;
      }
      i += 1;
    }
  }
  return res;
}

class State
{
  var type_cpp: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var all: dynamic = cpp_uninitialized();
  var lft: dynamic = cpp_uninitialized();
  var rgh: dynamic = cpp_uninitialized();
  func State(c: dynamic) -> dynamic
  {
      type_cpp = 0;
      cnt = 0;
      all += c;
      if ((all == pat))
      {
        cnt += 1;
      }
    }
  func State() -> dynamic
  {
      type_cpp = 0;
      cnt = 0;
    }
  func merge(oth: dynamic) -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      res.cnt = (cnt + oth.cnt);
      if ((pat.size() != 1))
      {
        res.type_cpp = 0;
        if ((type_cpp == 0))
        {
          if ((oth.type_cpp == 0))
          {
            res.cnt += calc(all, oth.all);
            res.all = (all + oth.all);
            if ((all.size() >= pat.size()))
            {
              res.type_cpp = 1;
              res.lft = res.all.substr(0, pat.size());
              res.rgh = res.all.substr((res.all.size() - pat.size()));
              res.all.clear();
            }
          } else
          {
            res.cnt += calc(all, oth.lft);
            res.type_cpp = 1;
            res.lft = ((all + oth.lft)).substr(0, pat.size());
            res.rgh = oth.rgh;
          }
        } else
        {
          if ((oth.type_cpp == 0))
          {
            res.cnt += calc(rgh, oth.all);
            res.type_cpp = 1;
            res.lft = lft;
            res.rgh = (rgh + oth.all);
            res.rgh = res.rgh.substr((res.rgh.size() - pat.size()));
          } else
          {
            res.cnt += calc(rgh, oth.lft);
            res.type_cpp = 1;
            res.lft = lft;
            res.rgh = oth.rgh;
          }
        }
      }
      ((*self)) = res;
    }
  func repeat(num: dynamic) -> dynamic
  {
      if ((num == 1))
      {
        return (*self);
      }
      num -= 1;
      var res: dynamic = cpp_construct((*self));
      var dbl: dynamic = cpp_construct((*self));
      while (num)
      {
        if ((num & 1))
        {
          res.merge(dbl);
        }
        dbl.merge(dbl);
        num >>= 1;
      }
      return res;
    }
  func State(c: dynamic, num: dynamic) -> dynamic
  {
      ((*self)) = State(c).repeat(num);
    }
}

var DEBUG: dynamic = 0;

func letter(s: dynamic, p: dynamic) -> dynamic
{
  if (DEBUG)
  {
    write(s, ":l:", p, "\n");
  }
  assert(((p < cpp_cast(s.size())) && isupper(s[p])));
  return State(s[cpp_update(p, "++")]);
}

func number(s: dynamic, p: dynamic) -> dynamic
{
  if (DEBUG)
  {
    write(s, ":n:", p, "\n");
  }
  var res: dynamic = 0;
  while (((p < cpp_cast(s.size())) && isdigit(s[p])))
  {
    res = ((res * 10) + ((s[p] - cpp_char("0"))));
    p += 1;
  }
  return res;
}

func expr(s: dynamic, p: dynamic) -> dynamic
{
  if (DEBUG)
  {
    write(s, ":e:", p, "\n");
  }
  var res: dynamic = factor(s, p);
  while (((p < cpp_cast(s.size())) && (s[p] != cpp_char(")"))))
  {
    var nxt: dynamic = factor(s, p);
    res.merge(nxt);
  }
  return res;
}

func factor(s: dynamic, p: dynamic) -> dynamic
{
  if (DEBUG)
  {
    write(s, ";f;", p, "\n");
  }
  if (isdigit(s[p]))
  {
    var num: dynamic = number(s, p);
    if (isupper(s[p]))
    {
      return State(s[cpp_update(p, "++")], num);
    }
    assert((s[p] == cpp_char("(")));
    p += 1;
    var res: dynamic = expr(s, p);
    assert((s[p] == cpp_char(")")));
    p += 1;
    return res.repeat(num);
  }
  return letter(s, p);
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  var s: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> s), (s != "#")))
  {
    read(pat);
    base = RH(pat);
    var p: dynamic = 0;
    var res: dynamic = expr(s, p);
    write(res.cnt, "\n");
  }
  return 0;
}
