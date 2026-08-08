// Translated from solution.cpp.

var L: dynamic;

var R: dynamic;

var M: dynamic;

var c = cpp_array(120000);

var d = cpp_array(120000);

var i: dynamic;

var n: dynamic;

var p: dynamic;

var s: dynamic;

var t1: dynamic;

var t2: dynamic;

var t3: dynamic;

var t4: dynamic;

func main()
{
  scanf("%s", (c + 1));
  n = strlen((c + 1));
  {
    var i = n;
    while ((i > 0))
    {
      if ((c[i] == cpp_char("L")))
      {
        t1 += 1;
        t3 = i;
        L.insert(i);
        M.insert(i);
      } else
      {
        t2 += 1;
        t4 = i;
        R.insert(i);
        M.insert(i);
      }
      i -= 1;
    }
  }
  L.insert((n + 1));
  R.insert((n + 1));
  if ((t1 == (t2 + 1)))
  {
    p = t3;
  } else if (((t1 + 1) == t2))
  {
    p = t4;
  } else
  {
    p = 1;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      d[i] = p;
      if ((c[p] == cpp_char("L")))
      {
        L.erase(p);
        M.erase(p);
        p = (*R.lower_bound(p));
        if ((p == (n + 1)))
        {
          p = (*R.begin());
        } else if (((c[(*M.begin())] == cpp_char("R")) && ((*L.lower_bound(p)) == (n + 1))))
        {
          p = (*M.begin());
        }
      } else
      {
        R.erase(p);
        M.erase(p);
        p = (*L.lower_bound(p));
        if ((p == (n + 1)))
        {
          p = (*L.begin());
        } else if (((c[(*M.begin())] == cpp_char("L")) && ((*R.lower_bound(p)) == (n + 1))))
        {
          p = (*M.begin());
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      if ((d[i] > d[(i + 1)]))
      {
        s += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", s);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d ", d[i]);
      i += 1;
    }
  }
}
