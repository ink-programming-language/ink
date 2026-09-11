// Translated from solution.cpp.

class p
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  func p(l: dynamic, r: dynamic, c: dynamic) -> dynamic
  {
      self->l = cpp_construct(l);
      self->r = cpp_construct(r);
      self->c = cpp_construct(c);
    }
}

func cmp(i: dynamic, j: dynamic) -> dynamic
{
  return (i.l < j.l);
}

var a: dynamic = cpp_array(200002);

var n: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(100001);

var r: dynamic = cpp_array(100001);

var d: dynamic = cpp_array(100001, 6);

func f(h: dynamic, k: dynamic) -> dynamic
{
  if ((k < 0))
  {
    return 1;
  }
  if ((d[h][k] >= 0))
  {
    return d[h][k];
  }
  if (((v[(k + 1)].l - h) <= v[k].r))
  {
    return cpp_assign(d[h][k], "=", 0);
  }
  {
    var i: dynamic = (v[k].l - 4);
    while ((i <= v[k].l))
    {
      {
        var j: dynamic = v[k].r;
        while ((j <= (v[k].r + 4)))
        {
          var cl: dynamic = i;
          var cr: dynamic = j;
          var nl: dynamic = (v[(k + 1)].l - h);
          if ((((cl < 1) || (cr > n)) || (cr >= nl)))
          {
            j += 1;
            continue;
          }
          if (((((cr - cl) + 1) > 5) || (((cr - cl) + 1) < 2)))
          {
            j += 1;
            continue;
          }
          var c1: dynamic = ((((nl - cr) + 3)) / 5);
          var c2: dynamic = ((((nl - cr) - 1)) / 2);
          if (((((v[k].c + c1) + 1) > v[(k + 1)].c) || (((v[k].c + c2) + 1) < v[(k + 1)].c)))
          {
            j += 1;
            continue;
          }
          if (f((v[k].l - i), (k - 1)))
          {
            return cpp_assign(d[h][k], "=", 1);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return cpp_assign(d[h][k], "=", 0);
}

func trace(h: dynamic, k: dynamic) -> dynamic
{
  if ((k < 0))
  {
    return;
  }
  {
    var i: dynamic = (v[k].l - 4);
    while ((i <= v[k].l))
    {
      {
        var j: dynamic = v[k].r;
        while ((j <= (v[k].r + 4)))
        {
          var cl: dynamic = i;
          var cr: dynamic = j;
          var nl: dynamic = (v[(k + 1)].l - h);
          if ((((cl < 1) || (cr > n)) || (cr >= nl)))
          {
            j += 1;
            continue;
          }
          if (((((cr - cl) + 1) > 5) || (((cr - cl) + 1) < 2)))
          {
            j += 1;
            continue;
          }
          var c1: dynamic = ((((nl - cr) + 3)) / 5);
          var c2: dynamic = ((((nl - cr) - 1)) / 2);
          if (((((v[k].c + c1) + 1) > v[(k + 1)].c) || (((v[k].c + c2) + 1) < v[(k + 1)].c)))
          {
            j += 1;
            continue;
          }
          if (f((v[k].l - i), (k - 1)))
          {
            {
              var p: dynamic = i;
              while ((p <= j))
              {
                a[p] = v[k].c;
                p += 1;
              }
            }
            var need: dynamic = (((v[(k + 1)].c - v[k].c) - 1));
            var len: dynamic = ((nl - cr) - 1);
            if (need)
            {
              var p: dynamic = (len / need);
              var q: dynamic = (len % need);
              var sum: dynamic = 0;
              var cur: dynamic = (v[k].c + 1);
              {
                var i: dynamic = (cr + 1);
                while ((i < nl))
                {
                  a[i] = cur;
                  sum += 1;
                  if ((sum == p))
                  {
                    if (q)
                    {
                      q -= 1;
                    } else
                    {
                      sum = 0;
                      cur += 1;
                    }
                  } else if ((sum > p))
                  {
                    sum = 0;
                    cur += 1;
                  }
                  i += 1;
                }
              }
            }
            trace((v[k].l - i), (k - 1));
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      if ((i == 1))
      {
        if ((a[i] > 1))
        {
          puts("-1");
          return 0;
        }
        a[i] = 1;
      }
      if (a[i])
      {
        if ((!l[a[i]]))
        {
          l[a[i]] = i;
        }
        r[a[i]] = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= 100000))
    {
      if (l[i])
      {
        v.push_back(p(l[i], r[i], i));
      }
      i += 1;
    }
  }
  if ((v.size() == 1))
  {
    if ((((v[0].r - v[0].l) + 1) > 5))
    {
      puts("-1");
      return 0;
    }
    if ((v[0].r == 1))
    {
      a[2] = 1;
      v[0].r += 1;
    }
    {
      var i: dynamic = v[0].l;
      while ((i <= v[0].r))
      {
        a[i] = 1;
        i += 1;
      }
    }
    var rem: dynamic = (((n - v[0].r)) / 2);
    var cur: dynamic = 2;
    {
      var i: dynamic = (v[0].r + 1);
      while ((i < n))
      {
        a[i] = cpp_assign(a[(i + 1)], "=", cur);
        cur += 1;
        i += 2;
      }
    }
    if ((a[n] == 0))
    {
      a[n] = a[(n - 1)];
    }
    printf("%d\n", a[n]);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        printf("%d ", a[i]);
        i += 1;
      }
    }
    puts("");
    return 0;
  }
  sort(v.begin(), v.end(), cmp);
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      if ((((v[i].r - v[i].l) + 1) > 5))
      {
        puts("-1");
        return 0;
      }
      if ((i && (v[i].l <= v[(i - 1)].r)))
      {
        puts("-1");
        return 0;
      }
      if ((i && (v[i].c < v[(i - 1)].c)))
      {
        puts("-1");
        return 0;
      }
      if ((((((i > 0) && (i < (v.size() - 1))) && (v[i].l == v[i].r)) && (v[(i - 1)].r == (v[i].l - 1))) && (v[(i + 1)].l == (v[i].r + 1))))
      {
        puts("-1");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      {
        var j: dynamic = v[i].l;
        while ((j <= v[i].r))
        {
          a[j] = v[i].c;
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(d, -1, cpp_sizeof((d)));
  {
    var l: dynamic = (v.back().l - 4);
    while ((l <= v.back().l))
    {
      {
        var r: dynamic = v.back().r;
        while ((r <= (v.back().r + 4)))
        {
          if (((((r - l) + 1) > 5) || (((r - l) + 1) < 2)))
          {
            r += 1;
            continue;
          }
          if (((l < 1) || (r > n)))
          {
            r += 1;
            continue;
          }
          if (f((v.back().l - l), (v.size() - 2)))
          {
            if (((r == (n - 1)) && (((r - l) + 1) == 5)))
            {
              r += 1;
              continue;
            }
            trace((v.back().l - l), (v.size() - 2));
            {
              var j: dynamic = l;
              while ((j <= r))
              {
                a[j] = v.back().c;
                j += 1;
              }
            }
            var cur: dynamic = (v.back().c + 1);
            {
              var j: dynamic = (r + 1);
              while ((j < n))
              {
                a[j] = cpp_assign(a[(j + 1)], "=", cur);
                cur += 1;
                j += 2;
              }
            }
            if ((a[n] == 0))
            {
              a[n] = a[(n - 1)];
            }
            printf("%d\n", a[n]);
            {
              var j: dynamic = 1;
              while ((j <= n))
              {
                printf("%d ", a[j]);
                j += 1;
              }
            }
            puts("");
            return 0;
          }
          r += 1;
        }
      }
      l += 1;
    }
  }
  puts("-1");
  return 0;
}
