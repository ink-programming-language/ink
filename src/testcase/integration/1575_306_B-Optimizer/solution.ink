// Translated from solution.cpp.

func gi(x: dynamic) -> dynamic
{
  var ch: dynamic = getchar();
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

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(202020);

var r: dynamic = cpp_array(202020);

var q: dynamic = cpp_array(202020);

func swap(i: dynamic, j: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
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

func qsort(l: dynamic, r: dynamic) -> dynamic
{
  var i: dynamic = l;
  var j: dynamic = r;
  var mid: dynamic = ((rand() % (((r - l) + 1))) + l);
  var ml: dynamic = l[mid];
  var mr: dynamic = r[mid];
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

var lx: dynamic = cpp_array(202020);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var L: dynamic = 0;
  var R: dynamic = 0;
  var s: dynamic = 0;
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
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
