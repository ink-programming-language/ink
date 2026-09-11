// Translated from solution.cpp.

var S: dynamic = cpp_array(1000002);

var D: dynamic = cpp_array(1000002);

var R: dynamic = cpp_array(1002, 1002);

func mS(s: dynamic, d: dynamic, o: dynamic, t: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var c: dynamic = ((o + t) >> 1);
  if ((o < c))
  {
    mS(d, s, o, c);
  }
  if (((c + 1) < t))
  {
    mS(d, s, (c + 1), t);
  }
  i = o;
  j = (c + 1);
  k = o;
  while (((i <= c) && (j <= t)))
  {
    if ((s[i] < s[j]))
    {
      d[k] = s[i];
      i += 1;
    } else
    {
      d[k] = s[j];
      j += 1;
    }
    k += 1;
  }
  while ((i <= c))
  {
    d[k] = s[i];
    i += 1;
    k += 1;
  }
  while ((j <= t))
  {
    d[k] = s[j];
    j += 1;
    k += 1;
  }
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  scanf("%d %d %d", (&n), (&l), (&k));
  scanf("%s", (&S));
  {
    i = 0;
    while (S[i])
    {
      D[i] = S[i];
      i += 1;
    }
  }
  mS(S, D, 0, (i - 1));
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 0;
        while ((j < l))
        {
          R[i][j] = cpp_char("*");
          j += 1;
        }
      }
      i += 1;
    }
  }
  t = (k - 1);
  C = k;
  {
    i = 0;
    while ((i < l))
    {
      {
        a = 0;
        while ((a < C))
        {
          R[(k - a)][i] = D[(t - a)];
          a += 1;
        }
      }
      temp = 1;
      {
        a = 1;
        while ((a < C))
        {
          if ((D[t] == D[(t - a)]))
          {
            temp += 1;
          } else
          {
            break;
          }
          a += 1;
        }
      }
      {
        a = 0;
        while ((a < C))
        {
          D[(t - a)] = cpp_char("*");
          a += 1;
        }
      }
      C = temp;
      t += C;
      i += 1;
    }
  }
  t = 0;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 0;
        while ((j < l))
        {
          if ((R[i][j] == cpp_char("*")))
          {
            while ((D[t] == cpp_char("*")))
            {
              t += 1;
            }
            R[i][j] = D[t];
            t += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      R[i][l] = cpp_char("\u{0}");
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      printf("%s\n", R[i]);
      i += 1;
    }
  }
  return 0;
}
