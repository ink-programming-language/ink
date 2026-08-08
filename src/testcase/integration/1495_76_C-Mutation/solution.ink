// Translated from solution.cpp.

func max(a: dynamic, b: dynamic)
{
  return if ((a < b)) b else a;
}

func min(a: dynamic, b: dynamic)
{
  return if ((a < b)) a else b;
}

var lf = cpp_char("\n");

var bufl = (1 << 15);

var buf = cpp_array(bufl);

var s = buf;

var t = buf;

func fetch()
{
  if ((s == t))
  {
    t = ((cpp_assign(s, "=", buf)) + fread(buf, 1, bufl, stdin));
    if ((s == t))
    {
      return EOF;
    }
  }
  return (*cpp_update(s, "++"));
}

func ty()
{
  var a = 0;
  var b = 1;
  var c = fetch();
  while ((!isdigit(c)))
  {
    b ^= (c == cpp_char("-"));
    c = fetch();
  }
  while (isdigit(c))
  {
    a = (((a * 10) + c) - 48);
    c = fetch();
  }
  return if (b) a else (-a);
}

func ts(s: dynamic)
{
  var a = 0;
  var c = fetch();
  while (((c <= 32) && (c != EOF)))
  {
    c = fetch();
  }
  while (((c > 32) && (c != EOF)))
  {
    s[cpp_update(a, "++")] = c;
    c = fetch();
  }
  s[a] = 0;
  return a;
}

var cpp_name = 200007;

var alp = 22;

var cpp_name = 4233333;

var n: dynamic;

var ps: dynamic;

var str = cpp_array(cpp_name);

var ned = cpp_array(alp);

var val = cpp_array(alp, alp);

var lim: dynamic;

var f = [0];

func main()
{
  ios.sync_with_stdio(0);
  cout.tie(null);
  n = ty();
  ps = ty();
  lim = ty();
  ts((str + 1));
  {
    var i = 0;
    while ((i < ps))
    {
      ned[i] = ty();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < ps))
    {
      {
        var j = 0;
        while ((j < ps))
        {
          val[i][j] = ty();
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      str[i] -= cpp_char("A");
      i += 1;
    }
  }
  var las = cpp_array(alp);
  memset(las, -1, cpp_sizeof((las)));
  var all = 0;
  {
    var i = 0;
    while ((i < ps))
    {
      f[((1 << (i)))] = ned[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      all |= ((1 << (str[i])));
      {
        var j = 0;
        while ((j < ps))
        {
          if ((las[j] < 0))
          {
            j += 1;
            continue;
          }
          if (((!(((((las[j]) >> (j))) & 1))) && (!(((((las[j]) >> (str[i]))) & 1)))))
          {
            f[las[j]] += val[j][str[i]];
            f[(las[j] | ((1 << (j))))] -= val[j][str[i]];
            f[(las[j] | ((1 << (str[i]))))] -= val[j][str[i]];
            f[((las[j] | ((1 << (j)))) | ((1 << (str[i]))))] += val[j][str[i]];
          }
          las[j] |= ((1 << (str[i])));
          j += 1;
        }
      }
      las[str[i]] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < alp))
    {
      {
        var j = 0;
        while ((j < ((1 << (alp)))))
        {
          if ((((((j) >> (i))) & 1)))
          {
            f[j] += f[(j - ((1 << (i))))];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < ((1 << (alp)))))
    {
      if ((((((i & all)) == i) && (i != all)) && (f[i] <= lim)))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, lf);
  return 0;
}
