// Translated from solution.cpp.

func gi(x: dynamic)
{
  var ch = getchar();
  x = 0;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    ch = getchar();
  }
  while ((!(((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
  {
    x = (((x * 10) + ch) - 48);
    ch = getchar();
  }
}

var n: dynamic;

var m: dynamic;

var l = cpp_array(202020);

var r = cpp_array(202020);

var q = cpp_array(202020);

func swap(i: dynamic, j: dynamic)
{
  var t: dynamic;
  t = l[i];
  l[i] = l[j];
  l[j] = t;
  t = r[i];
  r[i] = r[j];
  r[j] = t;
  t = q[i];
  q[i] = q[j];
  q[j] = t;
}

func qsort(l: dynamic, r: dynamic)
{
  var i = l;
  var j = r;
  var mid = ((rand() % (((r - l) + 1))) + l);
  var ml = l[mid];
  var mr = r[mid];
  while ((i <= j))
  {
    {
      while (((l[i] < ml) || (((l[i] == ml) && (r[i] > mr)))))
      {
        i += 1;
      }
    }
    {
      while (((l[j] > ml) || (((l[j] == ml) && (r[j] < mr)))))
      {
        j -= 1;
      }
    }
    if ((i <= j))
    {
      swap(cpp_update(i, "++"), cpp_update(j, "--"));
    }
  }
  if ((l < j))
  {
    qsort(l, j);
  }
  if ((i < r))
  {
    qsort(i, r);
  }
}

var lx = cpp_array(202020);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var L = 0;
  var R = 0;
  var s = 0;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  gi(n);
  gi(m);
  {
    i = 1;
    while ((i <= m))
    {
      gi(a);
      gi(b);
      l[i] = a;
      r[i] = ((a + b) - 1);
      q[i] = i;
      i += 1;
    }
  }
  qsort(1, m);
  j = 1;
  {
    i = 1;
    while ((i <= m))
    {
      if ((l[i] > (R + 1)))
      {
        L = l[i];
        R = r[i];
        lx[i] = 1;
        i += 1;
      } else
      {
        a = R;
        b = 0;
        {
          while (((j <= m) && (l[j] <= (R + 1))))
          {
            if ((r[j] > a))
            {
              a = r[j];
              b = j;
            }
            j += 1;
          }
        }
        if ((a > R))
        {
          R = a;
        }
        s += 1;
        lx[b] = 1;
        i = j;
      }
    }
  }
  s = 0;
  {
    i = 1;
    while ((i <= m))
    {
      if (lx[i])
      {
        s += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", int_cpp((m - s)));
  {
    i = 1;
    while ((i <= m))
    {
      l[q[i]] = i;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      if ((!lx[l[i]]))
      {
        printf("%d ", cpp_cast(i));
      }
      i += 1;
    }
  }
  return 0;
}
