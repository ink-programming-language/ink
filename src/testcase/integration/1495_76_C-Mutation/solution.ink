// Translated from solution.cpp.

func max(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? b : a;
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a < b)) ? a : b;
}

var lf: dynamic = cpp_char("\n");

var bufl: dynamic = (1 << 15);

var buf: dynamic = cpp_array(bufl);

var s: dynamic = buf;

var t: dynamic = buf;

func fetch() -> dynamic
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

func ty() -> dynamic
{
  var a: dynamic = 0;
  var b: dynamic = 1;
  var c: dynamic = fetch();
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
  return  (b) ? a : (-a);
}

func ts(s: dynamic) -> dynamic
{
  var a: dynamic = 0;
  var c: dynamic = fetch();
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

var cpp_name: dynamic = 200007;

var alp: dynamic = 22;

var cpp_name: dynamic = 4233333;

var n: dynamic = cpp_uninitialized();

var ps: dynamic = cpp_uninitialized();

var str: dynamic = cpp_array(cpp_name);

var ned: dynamic = cpp_array(alp);

var val: dynamic = cpp_array(alp, alp);

var lim: dynamic = cpp_uninitialized();

var f: dynamic = [0];

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cout.tie(null);
  n = ty();
  ps = ty();
  lim = ty();
  ts((str + 1));
  {
    var i: dynamic = 0;
    while ((i < ps))
    {
      ned[i] = ty();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < ps))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      str[i] -= cpp_char("A");
      i += 1;
    }
  }
  var las: dynamic = cpp_array(alp);
  memset(las, -1, cpp_sizeof((las)));
  var all: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < ps))
    {
      f[((1 << (i)))] = ned[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      all |= ((1 << (str[i])));
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < alp))
    {
      {
        var j: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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
