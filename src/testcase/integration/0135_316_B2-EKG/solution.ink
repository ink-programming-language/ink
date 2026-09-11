// Translated from solution.cpp.

var s: dynamic = cpp_array(5555);

var b: dynamic = cpp_array(5555);

var mark: dynamic = cpp_array(5555);

var moze: dynamic = cpp_array(5555);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var br: dynamic = cpp_uninitialized();
  var of: dynamic = cpp_uninitialized();
  var bb: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&x));
  {
    i = 0;
    while ((i <= n))
    {
      mark[i] = false;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&s[i]));
      mark[s[i]] = true;
      i += 1;
    }
  }
  br = 0;
  of = 0;
  q = -1;
  {
    i = 1;
    while ((i <= n))
    {
      if ((!mark[i]))
      {
        t = i;
        bb = 0;
        b[br] = 0;
        while ((t != 0))
        {
          b[br] += 1;
          if (bb)
          {
            of += 1;
          }
          if ((t == x))
          {
            of += 1;
            bb = 1;
          }
          t = s[t];
        }
        if (bb)
        {
          q = br;
        }
        br += 1;
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= 1555))
    {
      moze[i] = false;
      i += 1;
    }
  }
  moze[of] = true;
  {
    i = 0;
    while ((i < br))
    {
      if ((i != q))
      {
        {
          j = 1234;
          while ((j > b[i]))
          {
            if (moze[(j - b[i])])
            {
              moze[j] = true;
            }
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= 1234))
    {
      if (moze[i])
      {
        printf("%d\n", i);
      }
      i += 1;
    }
  }
  return 0;
}
