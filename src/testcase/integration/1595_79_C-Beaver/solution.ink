// Translated from solution.cpp.

func print(v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      write(v[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
}

func z_function(s: dynamic)
{
  var n = cpp_cast(s.length());
  {
    var i = 1;
    var l = 0;
    var r = 0;
    while ((i < n))
    {
      if ((i <= r))
      {
        z[i] = min(((r - i) + 1), z[(i - l)]);
      }
      while ((((i + z[i]) < n) && (s[z[i]] == s[(i + z[i])])))
      {
        z[i] += 1;
      }
      if ((((i + z[i]) - 1) > r))
      {
        l = i;
        r = ((i + z[i]) - 1);
      }
      i += 1;
    }
  }
  return z;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var s: dynamic;
  read(s);
  var n = s.length();
  var q: dynamic;
  read(q);
  var sr = s;
  reverse(sr.begin(), sr.end());
  var v = cpp_construct(n, (n + 1));
  while (cpp_update(q, "--"))
  {
    var t: dynamic;
    read(t);
    var l = t.length();
    {
      var i = 0;
      while ((i < n))
      {
        if (((i + l) > n))
        {
          break;
        }
        if ((s.substr(i, l) == t))
        {
          v[((i + l) - 1)] = min(v[((i + l) - 1)], l);
        }
        i += 1;
      }
    }
  }
  var maxlen = if (((v[0] == 1))) 0 else 1;
  var index = 0;
  var ans = maxlen;
  {
    var i = 1;
    while ((i < n))
    {
      var len = min((v[i] - 1), (ans + 1));
      if ((len > maxlen))
      {
        maxlen = len;
        index = ((i - len) + 1);
      }
      ans = len;
      i += 1;
    }
  }
  write(maxlen, cpp_char(" "), index, cpp_char("\n"));
}
