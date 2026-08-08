// Translated from solution.cpp.

var N = (5e6 + 10);

var n: dynamic;

var k: dynamic;

var m: dynamic;

var A = cpp_array(N);

var c = cpp_array(N);

var tlen: dynamic;

var z = cpp_array((N << 1));

var s = cpp_array(N);

var t = cpp_array(N);

var S = cpp_array(N);

var T = cpp_array(N);

var SS = cpp_array((N << 1));

func check(len: dynamic)
{
  {
    var i = 1;
    while ((i <= len))
    {
      if ((T[i] < S[i]))
      {
        return;
      }
      if ((T[i] > S[i]))
      {
        break;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= len))
    {
      T[i] = S[i];
      i += 1;
    }
  }
  return;
}

func main()
{
  scanf("%s%d", (s + 1), (&k));
  n = strlen((s + 1));
  reverse((s + 1), ((s + n) + 1));
  if ((k == 1))
  {
    {
      var i = 1;
      while ((i <= n))
      {
        t[i] = s[((n - i) + 1)];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= n))
      {
        if ((s[i] < t[i]))
        {
          return cpp_comma(printf("%s", (s + 1)), 0);
        }
        if ((t[i] < s[i]))
        {
          return cpp_comma(printf("%s", (t + 1)), 0);
        }
        i += 1;
      }
    }
    return cpp_comma(printf("%s", (s + 1)), 0);
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var j = i;
      var k = (i + 1);
      while (((k <= n) && (s[j] <= s[k])))
      {
        if ((s[j] < s[k]))
        {
          j = i;
        } else
        {
          j += 1;
        }
        k += 1;
      }
      A[cpp_update(m, "++")] = i;
      c[m] = (k - j);
      while ((i <= j))
      {
        i += (k - j);
      }
    }
  }
  A[(m + 1)] = (n + 1);
  while (((m > 0) && (k >= 3)))
  {
    {
      var i = A[m];
      while ((i <= (A[(m + 1)] - 1)))
      {
        t[cpp_update(tlen, "++")] = s[i];
        i += 1;
      }
    }
    if (((c[m] != 1) || (c[(m - 1)] != 1)))
    {
      k -= 1;
    }
    m -= 1;
  }
  if ((m == 0))
  {
    return cpp_comma(printf("%s", (t + 1)), 0);
  }
  T[1] = (cpp_char("z") + 1);
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      S[i] = s[(A[(m + 1)] - i)];
      i += 1;
    }
  }
  check((A[(m + 1)] - 1));
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      SS[i] = cpp_assign(SS[((i + A[(m + 1)]) - 1)], "=", s[i]);
      i += 1;
    }
  }
  var x = 1;
  var y = 2;
  var k = 0;
  {
    while ((((x <= (A[(m + 1)] - 1)) && (y <= (A[(m + 1)] - 1))) && (k <= (A[(m + 1)] - 2))))
    {
      if ((SS[(x + k)] == SS[(y + k)]))
      {
        k += 1;
      } else if ((SS[(x + k)] < SS[(y + k)]))
      {
        y += (k + 1);
        k = 0;
      } else
      {
        x += (k + 1);
        k = 0;
      }
      if ((x == y))
      {
        y += 1;
      }
    }
  }
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      S[i] = SS[((min(x, y) + i) - 1)];
      i += 1;
    }
  }
  check((A[(m + 1)] - 1));
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      SS[i] = s[i];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      SS[((i + A[(m + 1)]) - 1)] = s[(A[(m + 1)] - i)];
      i += 1;
    }
  }
  var len = (2 * ((A[(m + 1)] - 1)));
  {
    var i = 2;
    var mr = 1;
    var ml: dynamic;
    while ((i <= len))
    {
      z[i] = (if ((i < mr)) min(z[((i - ml) + 1)], (mr - i)) else 0);
      while ((SS[(z[i] + 1)] == SS[(i + z[i])]))
      {
        z[i] += 1;
      }
      if (((i + z[i]) > mr))
      {
        mr = (i + z[i]);
        ml = i;
      }
      i += 1;
    }
  }
  z[1] = len;
  k = A[(m + 1)];
  {
    var i = (A[(m + 1)] - 1);
    while ((i >= 1))
    {
      var l = (A[(m + 1)] - i);
      var r = ((A[(m + 1)] - k) + 1);
      var op = z[((r + A[(m + 1)]) - 1)];
      if ((op < ((l - r) + 1)))
      {
        if ((SS[(op + 1)] > SS[((k - op) - 1)]))
        {
          k = i;
        }
      } else
      {
        op = (k - i);
        l = (op + 1);
        ((r - k) - 1);
        if ((SS[(z[l] + 1)] < SS[(z[l] + l)]))
        {
          k = i;
        }
      }
      i -= 1;
    }
  }
  var tot = 0;
  {
    var i = (A[(m + 1)] - 1);
    while ((i >= k))
    {
      S[cpp_update(tot, "++")] = SS[i];
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= (k - 1)))
    {
      S[cpp_update(tot, "++")] = SS[i];
      i += 1;
    }
  }
  check((A[(m + 1)] - 1));
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      SS[i] = s[i];
      i += 1;
    }
  }
  var p = m;
  while (((((A[(p + 1)] - A[p])) * 2) <= (((A[p] - A[(p - 1)])) + 1)))
  {
    var flag = 0;
    {
      var i = (A[p] - 1);
      while ((i >= A[(p - 1)]))
      {
        if ((SS[i] < SS[(((A[(m + 1)] - 1) - i) + A[(p - 1)])]))
        {
          flag = 1;
          break;
        }
        if ((SS[i] > SS[(((A[(m + 1)] - 1) - i) + A[(p - 1)])]))
        {
          break;
        }
        i -= 1;
      }
    }
    if (flag)
    {
      break;
    }
    p -= 1;
  }
  p = A[p];
  tot = 0;
  {
    var i = p;
    while ((i <= (A[(m + 1)] - 1)))
    {
      S[cpp_update(tot, "++")] = SS[i];
      i += 1;
    }
  }
  {
    var i = (p - 1);
    while ((i >= 1))
    {
      S[cpp_update(tot, "++")] = SS[i];
      i -= 1;
    }
  }
  check((A[(m + 1)] - 1));
  {
    var i = 1;
    while ((i <= (A[(m + 1)] - 1)))
    {
      t[cpp_update(tlen, "++")] = T[i];
      i += 1;
    }
  }
  printf("%s", (t + 1));
  return 0;
}
