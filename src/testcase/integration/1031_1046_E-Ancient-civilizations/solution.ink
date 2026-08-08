// Translated from solution.cpp.

class point
{
  var x: dynamic;
  var y: dynamic;
  var op: dynamic;
  var id: dynamic;
}

var p = cpp_array(1100);

func multi(p1: dynamic, p2: dynamic, p0: dynamic)
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  x1 = (p1.x - p0.x);
  y1 = (p1.y - p0.y);
  x2 = (p2.x - p0.x);
  y2 = (p2.y - p0.y);
  return ((x1 * y2) - (x2 * y1));
}

func cmp(p1: dynamic, p2: dynamic)
{
  return (multi(p1, p2, p[1]) > 0);
}

var aslen: dynamic;

var asx = cpp_array(1100);

var asy = cpp_array(1100);

func pb(x: dynamic, y: dynamic)
{
  asx[cpp_update(aslen, "++")] = x;
  asy[aslen] = y;
}

var n: dynamic;

var top: dynamic;

var sta = cpp_array(1100);

var insta = cpp_array(1100);

func in_triangle(p1: dynamic, p2: dynamic, p3: dynamic, p0: dynamic)
{
  var t1 = multi(p1, p0, p2);
  var t2 = multi(p2, p0, p3);
  var t3 = multi(p3, p0, p1);
  if ((((((t1 < 0) && (t2 < 0)) && (t3 < 0))) || ((((t1 > 0) && (t2 > 0)) && (t3 > 0)))))
  {
    return true;
  }
  return false;
}

func separate(p1: dynamic, p2: dynamic, p3: dynamic, L: dynamic, R: dynamic)
{
  if ((L > R))
  {
    return;
  }
  var k = -1;
  {
    var i = L;
    while ((i <= R))
    {
      if ((p[i].op != p1.op))
      {
        k = R;
        if ((insta[R] != 0))
        {
          insta[i] = insta[R];
          sta[insta[i]] = i;
          insta[R] = 0;
        }
        swap(p[R], p[i]);
        break;
      }
      i += 1;
    }
  }
  pb(p3.id, p[k].id);
  var l = L;
  var r = (L - 1);
  var bk = false;
  {
    var i = l;
    while ((i <= R))
    {
      if ((i != k))
      {
        if (in_triangle(p1, p2, p[k], p[i]))
        {
          if ((p[i].op != p1.op))
          {
            bk = true;
          }
          r += 1;
          if ((insta[r] != 0))
          {
            insta[i] = insta[r];
            sta[insta[i]] = i;
            insta[r] = 0;
          }
          swap(p[r], p[i]);
        }
      }
      i += 1;
    }
  }
  if ((bk == false))
  {
    {
      var i = l;
      while ((i <= r))
      {
        pb(p1.id, p[i].id);
        if ((p1.op != p[i].op))
        {
          printf("error 1\n");
        }
        i += 1;
      }
    }
  } else
  {
    separate(p1, p2, p[k], l, r);
  }
  l = (r + 1);
  bk = false;
  {
    var i = l;
    while ((i <= R))
    {
      if ((i != k))
      {
        if (in_triangle(p1, p3, p[k], p[i]))
        {
          if ((p[i].op != p3.op))
          {
            bk = true;
          }
          r += 1;
          if ((insta[r] != 0))
          {
            insta[i] = insta[r];
            sta[insta[i]] = i;
            insta[r] = 0;
          }
          swap(p[r], p[i]);
        }
      }
      i += 1;
    }
  }
  if ((bk == false))
  {
    {
      var i = l;
      while ((i <= r))
      {
        pb(p3.id, p[i].id);
        if ((p3.op != p[i].op))
        {
          printf("error 2\n");
        }
        i += 1;
      }
    }
  } else
  {
    separate(p3, p[k], p1, l, r);
  }
  l = (r + 1);
  bk = false;
  {
    var i = l;
    while ((i <= R))
    {
      if ((i != k))
      {
        if (in_triangle(p2, p3, p[k], p[i]))
        {
          if ((p[i].op != p3.op))
          {
            bk = true;
          }
          r += 1;
          if ((insta[r] != 0))
          {
            insta[i] = insta[r];
            sta[insta[i]] = i;
            insta[r] = 0;
          }
          swap(p[r], p[i]);
        }
      }
      i += 1;
    }
  }
  if ((bk == false))
  {
    {
      var i = l;
      while ((i <= r))
      {
        pb(p3.id, p[i].id);
        if ((p3.op != p[i].op))
        {
          printf("error 3\n");
        }
        i += 1;
      }
    }
  } else
  {
    separate(p3, p[k], p2, l, r);
  }
}

func graham()
{
  top = 0;
  sta[cpp_update(top, "++")] = 1;
  sta[cpp_update(top, "++")] = 2;
  memset(insta, 0, cpp_sizeof((insta)));
  insta[1] = 1;
  insta[2] = 2;
  {
    var i = 3;
    while ((i <= n))
    {
      while (((top > 1) && (multi(p[sta[top]], p[i], p[sta[(top - 1)]]) <= 0)))
      {
        insta[sta[top]] = 0;
        top -= 1;
      }
      sta[cpp_update(top, "++")] = i;
      insta[sta[top]] = top;
      i += 1;
    }
  }
  var s = 0;
  {
    var i = 2;
    while ((i <= top))
    {
      s += ((p[sta[(i - 1)]].op ^ p[sta[i]].op));
      i += 1;
    }
  }
  s += ((p[sta[1]].op ^ p[sta[top]].op));
  if ((s > 2))
  {
    printf("Impossible\n");
  } else if ((s == 0))
  {
    var cc = p[sta[1]].op;
    var k = -1;
    {
      var i = 1;
      while ((i <= n))
      {
        if (((insta[i] == 0) && (p[i].op != cc)))
        {
          k = n;
          if ((insta[n] != 0))
          {
            insta[i] = insta[n];
            sta[insta[i]] = i;
            insta[n] = 0;
          }
          swap(p[n], p[i]);
          break;
        }
        i += 1;
      }
    }
    var L = 1;
    var R = 0;
    {
      var i = 2;
      while ((i <= top))
      {
        pb(p[sta[(i - 1)]].id, p[sta[i]].id);
        var bk = false;
        {
          var j = L;
          while ((j <= n))
          {
            if (((insta[j] == 0) && (j != k)))
            {
              if (((k == -1) || in_triangle(p[sta[(i - 1)]], p[sta[i]], p[k], p[j])))
              {
                if ((p[j].op != cc))
                {
                  bk = true;
                }
                R += 1;
                if ((insta[R] != 0))
                {
                  insta[j] = insta[R];
                  sta[insta[j]] = j;
                  insta[R] = 0;
                }
                swap(p[R], p[j]);
              }
            }
            j += 1;
          }
        }
        if ((bk == false))
        {
          {
            var j = L;
            while ((j <= R))
            {
              pb(p[sta[i]].id, p[j].id);
              j += 1;
            }
          }
        } else
        {
          separate(p[sta[(i - 1)]], p[sta[i]], p[k], L, R);
        }
        L = (R + 1);
        i += 1;
      }
    }
    var bk = false;
    {
      var j = L;
      while ((j <= n))
      {
        if (((insta[j] == 0) && (j != k)))
        {
          if (((k == -1) || in_triangle(p[sta[top]], p[sta[1]], p[k], p[j])))
          {
            if ((p[j].op != cc))
            {
              bk = true;
            }
            R += 1;
            if ((insta[R] != 0))
            {
              insta[j] = insta[R];
              sta[insta[j]] = j;
              insta[R] = 0;
            }
            swap(p[R], p[j]);
          }
        }
        j += 1;
      }
    }
    if ((bk == false))
    {
      {
        var j = L;
        while ((j <= R))
        {
          pb(p[sta[top]].id, p[j].id);
          j += 1;
        }
      }
    } else
    {
      separate(p[sta[top]], p[sta[1]], p[k], L, R);
    }
  } else
  {
    var be: dynamic;
    var bk = false;
    {
      var i = 1;
      while ((i <= top))
      {
        if ((p[sta[i]].op == 0))
        {
          if ((bk == false))
          {
            be = i;
            bk = true;
          }
        } else
        {
          bk = false;
        }
        i += 1;
      }
    }
    var L = 1;
    var R = 0;
    var i: dynamic;
    var k = ((((((be - 1) + top) - 1)) % top) + 1);
    {
      i = ((be % top) + 1);
      while ((p[sta[i]].op == 0))
      {
        var u = ((((((i - 1) + top) - 1)) % top) + 1);
        pb(p[sta[u]].id, p[sta[i]].id);
        var bk = false;
        {
          var j = L;
          while ((j <= n))
          {
            if (((insta[j] == false) && in_triangle(p[sta[u]], p[sta[i]], p[sta[k]], p[j])))
            {
              if ((p[j].op != 0))
              {
                bk = true;
              }
              R += 1;
              if ((insta[R] != 0))
              {
                insta[j] = insta[R];
                sta[insta[j]] = j;
                insta[R] = 0;
              }
              swap(p[R], p[j]);
            }
            j += 1;
          }
        }
        if ((bk == false))
        {
          {
            var j = L;
            while ((j <= R))
            {
              pb(p[sta[i]].id, p[j].id);
              j += 1;
            }
          }
        } else
        {
          separate(p[sta[u]], p[sta[i]], p[sta[k]], L, R);
        }
        L = (R + 1);
        i = ((i % top) + 1);
      }
    }
    k = ((((((i - 1) + top) - 1)) % top) + 1);
    {
      i = ((i % top) + 1);
      while ((i != be))
      {
        var u = ((((((i - 1) + top) - 1)) % top) + 1);
        pb(p[sta[u]].id, p[sta[i]].id);
        var bk = false;
        {
          var j = L;
          while ((j <= n))
          {
            if (((insta[j] == false) && in_triangle(p[sta[u]], p[sta[i]], p[sta[k]], p[j])))
            {
              if ((p[j].op != 1))
              {
                bk = true;
              }
              R += 1;
              if ((insta[R] != 0))
              {
                insta[j] = insta[R];
                sta[insta[j]] = j;
                insta[R] = 0;
              }
              swap(p[R], p[j]);
            }
            j += 1;
          }
        }
        if ((bk == false))
        {
          {
            var j = L;
            while ((j <= R))
            {
              pb(p[sta[i]].id, p[j].id);
              j += 1;
            }
          }
        } else
        {
          separate(p[sta[u]], p[sta[i]], p[sta[k]], L, R);
        }
        L = (R + 1);
        i = ((i % top) + 1);
      }
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d%d", (&p[i].x), (&p[i].y), (&p[i].op));
      p[i].id = (i - 1);
      if (((p[i].y < p[1].y) || (((p[i].y == p[1].y) && (p[i].x < p[1].x)))))
      {
        swap(p[i], p[1]);
      }
      i += 1;
    }
  }
  if ((n == 1))
  {
    printf("0\n");
    return 0;
  } else if ((n == 2))
  {
    if ((p[1].op == p[2].op))
    {
      printf("1\n0 1\n");
    } else
    {
      printf("0\n");
    }
    return 0;
  }
  sort((p + 2), ((p + n) + 1), cmp);
  aslen = 0;
  graham();
  if ((aslen != 0))
  {
    printf("%d\n", aslen);
    {
      var i = 1;
      while ((i <= aslen))
      {
        printf("%d %d\n", asx[i], asy[i]);
        i += 1;
      }
    }
  }
  return 0;
}
