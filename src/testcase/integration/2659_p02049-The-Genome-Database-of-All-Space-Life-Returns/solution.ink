// Translated from solution.cpp.

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func __cpp_top_level_1()
{
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func __cpp_top_level_2()
{
}

class RollingHash
{
  var hash: dynamic;
  var p: dynamic;
  func RollingHash()
  {
    }
  func RollingHash(s: dynamic)
  {
      var n = s.size();
      hash.assign((n + 1), 0);
      p.assign((n + 1), 1);
      {
        var i = 0;
        while ((i < n))
        {
          hash[(i + 1)] = ((((hash[i] * B) + s[i])) % MOD);
          p[(i + 1)] = ((p[i] * B) % MOD);
          i += 1;
        }
      }
    }
  func find(l: dynamic, r: dynamic)
  {
      var res = ((hash[r] + MOD) - ((hash[l] * p[(r - l)]) % MOD));
      return if ((res >= MOD)) (res - MOD) else res;
    }
}

var MOD = (1e9 + 7);

var B = 1777771;

var pat: dynamic;

var base: dynamic;

func calc(s: dynamic, t: dynamic)
{
  var res = 0;
  var n = s.size();
  var m = t.size();
  var len = pat.size();
  if (((n + m) < len))
  {
    return 0;
  }
  var rh = cpp_construct((s + t));
  {
    var i = 0;
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
  var type_cpp: dynamic;
  var cnt: dynamic;
  var all: dynamic;
  var lft: dynamic;
  var rgh: dynamic;
  func State(c: dynamic)
  {
      type_cpp = 0;
      cnt = 0;
      all += c;
      if ((all == pat))
      {
        cnt += 1;
      }
    }
  func State()
  {
      type_cpp = 0;
      cnt = 0;
    }
  func merge(oth: dynamic)
  {
      var res: dynamic;
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
      ((*this)) = res;
    }
  func repeat(num: dynamic)
  {
      if ((num == 1))
      {
        return (*this);
      }
      num -= 1;
      var res = cpp_construct((*this));
      var dbl = cpp_construct((*this));
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
  func State(c: dynamic, num: dynamic)
  {
      ((*this)) = State(c).repeat(num);
    }
}

var DEBUG = 0;

func letter(s: dynamic, p: dynamic)
{
  if (DEBUG)
  {
    write(s, ":l:", p, "\n");
  }
  assert(((p < cpp_cast(s.size())) && isupper(s[p])));
  return State(s[cpp_update(p, "++")]);
}

func number(s: dynamic, p: dynamic)
{
  if (DEBUG)
  {
    write(s, ":n:", p, "\n");
  }
  var res = 0;
  while (((p < cpp_cast(s.size())) && isdigit(s[p])))
  {
    res = ((res * 10) + ((s[p] - cpp_char("0"))));
    p += 1;
  }
  return res;
}

func expr(s: dynamic, p: dynamic)
{
  if (DEBUG)
  {
    write(s, ":e:", p, "\n");
  }
  var res = factor(s, p);
  while (((p < cpp_cast(s.size())) && (s[p] != cpp_char(")"))))
  {
    var nxt = factor(s, p);
    res.merge(nxt);
  }
  return res;
}

func factor(s: dynamic, p: dynamic)
{
  if (DEBUG)
  {
    write(s, ";f;", p, "\n");
  }
  if (isdigit(s[p]))
  {
    var num = number(s, p);
    if (isupper(s[p]))
    {
      return State(s[cpp_update(p, "++")], num);
    }
    assert((s[p] == cpp_char("(")));
    p += 1;
    var res = expr(s, p);
    assert((s[p] == cpp_char(")")));
    p += 1;
    return res.repeat(num);
  }
  return letter(s, p);
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(0);
  var s: dynamic;
  while (cpp_comma((cin >> s), (s != "#")))
  {
    read(pat);
    base = RH(pat);
    var p = 0;
    var res = expr(s, p);
    write(res.cnt, "\n");
  }
  return 0;
}
