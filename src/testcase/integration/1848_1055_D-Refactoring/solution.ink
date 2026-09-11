// Translated from solution.cpp.

var cur: dynamic = cpp_uninitialized();

var res: dynamic = cpp_uninitialized();

var w: dynamic = cpp_array(3005);

var k: dynamic = cpp_array(3005);

var stt: dynamic = cpp_array(3005);

var ed: dynamic = cpp_array(3005);

var f: dynamic = cpp_array(3005);

var tmp: dynamic = cpp_array(3005);

func build(s: dynamic) -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i < cpp_cast(s.length())))
    {
      f[i] = f[(i - 1)];
      while ((s[f[i]] != s[i]))
      {
        if ((!f[i]))
        {
          break;
        }
        f[i] = f[(f[i] - 1)];
      }
      if ((s[f[i]] == s[i]))
      {
        f[i] += 1;
      }
      i += 1;
    }
  }
}

func kmp(s: dynamic, patt: dynamic) -> dynamic
{
  var src: dynamic = cpp_array(s.length());
  var res: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(s.length())))
    {
      if ((!i))
      {
        src[i] = (s[i] == patt[i]);
        if ((src[i] == patt.length()))
        {
          res = i;
          break;
        }
        i += 1;
        continue;
      }
      src[i] = src[(i - 1)];
      while ((s[i] != patt[src[i]]))
      {
        if ((!src[i]))
        {
          break;
        }
        src[i] = f[(src[i] - 1)];
      }
      if ((s[i] == patt[src[i]]))
      {
        src[i] += 1;
      }
      if ((src[i] == patt.length()))
      {
        res = i;
        break;
      }
      i += 1;
    }
  }
  return res;
}

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", (&tmp));
      w[i] = tmp;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%s", (&tmp));
      k[i] = tmp;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      s = w[i];
      t = k[i];
      var fi: dynamic = -1;
      var se: dynamic = -1;
      {
        var j: dynamic = 0;
        while ((j < cpp_cast(s.length())))
        {
          if ((s[j] != t[j]))
          {
            se = j;
            if ((!((~fi))))
            {
              fi = j;
            }
          }
          j += 1;
        }
      }
      if ((!((~fi))))
      {
        stt[i] = -1;
        ed[i] = -1;
        i += 1;
        continue;
      }
      stt[i] = fi;
      ed[i] = se;
      if ((!cur.length()))
      {
        cur = s.substr(fi, ((se - fi) + 1));
        res = t.substr(fi, ((se - fi) + 1));
      }
      if ((((cur != s.substr(fi, ((se - fi) + 1)))) || ((res != t.substr(fi, ((se - fi) + 1))))))
      {
        printf("NO");
        return 0;
      }
      i += 1;
    }
  }
  while (1)
  {
    var h: dynamic = -1;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((!((~stt[i]))))
        {
          i += 1;
          continue;
        }
        if ((!stt[i]))
        {
          h = 0;
          break;
        }
        if ((!((~h))))
        {
          h = (w[i][(stt[i] - 1)] - cpp_char("0"));
        }
        if ((h != (w[i][(stt[i] - 1)] - cpp_char("0"))))
        {
          h = 0;
          break;
        }
        i += 1;
      }
    }
    if ((!h))
    {
      break;
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((!((~stt[i]))))
        {
          i += 1;
          continue;
        }
        stt[i] -= 1;
        i += 1;
      }
    }
  }
  while (1)
  {
    var h: dynamic = -1;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((!((~ed[i]))))
        {
          i += 1;
          continue;
        }
        if ((ed[i] == (w[i].size() - 1)))
        {
          h = 0;
          break;
        }
        if ((!((~h))))
        {
          h = (w[i][(ed[i] + 1)] - cpp_char("0"));
        }
        if ((h != (w[i][(ed[i] + 1)] - cpp_char("0"))))
        {
          h = 0;
          break;
        }
        i += 1;
      }
    }
    if ((!h))
    {
      break;
    }
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((!((~ed[i]))))
        {
          i += 1;
          continue;
        }
        ed[i] += 1;
        i += 1;
      }
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((stt[i] != -1))
      {
        cur = w[i].substr(stt[i], ((ed[i] - stt[i]) + 1));
        res = k[i].substr(stt[i], ((ed[i] - stt[i]) + 1));
        break;
      }
      i += 1;
    }
  }
  build(cur);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var k: dynamic = kmp(w[i], cur);
      if ((((((stt[i] == -1)) && ((k >= 0)))) || ((((stt[i] >= 0)) && ((k == -1))))))
      {
        printf("NO");
        return 0;
      }
      if ((k != ed[i]))
      {
        printf("NO");
        return 0;
      }
      i += 1;
    }
  }
  printf("YES\n%s\n%s", cur.c_str(), res.c_str());
  return 0;
}
