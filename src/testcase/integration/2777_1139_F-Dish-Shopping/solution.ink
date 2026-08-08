// Translated from solution.cpp.

func read(x: dynamic)
{
  var v = 0;
  var f = 1;
  var c = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func read(x: dynamic)
{
  var v = 0;
  var f = 1;
  var c = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func readc(x: dynamic)
{
  var c: dynamic;
  while (((cpp_assign(c, "=", getchar())) == cpp_char(" ")))
  {
  }
  x = c;
}

func writes(s: dynamic)
{
  puts(s.c_str());
}

func writeln()
{
  writes("");
}

func writei(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = abs(x);
  }
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a = cpp_array(25);
  var top = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

func writell(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = abs(x);
  }
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a = cpp_array(25);
  var top = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

var n: dynamic;

var m: dynamic;

var i: dynamic;

var j: dynamic;

var s = cpp_array(800005);

var ans = cpp_array(200005);

class ii
{
  var l: dynamic;
  var r: dynamic;
  var lys: dynamic;
  var inc: dynamic;
  var pref: dynamic;
  var op: dynamic;
}

var a = cpp_array(200005);

func cmp(x: dynamic, y: dynamic)
{
  if (((x.op == 0) && (y.op == 0)))
  {
    return (x.r > y.r);
  }
  if (((x.op == 0) && (y.op != 0)))
  {
    return (x.r >= y.inc);
  }
  if (((x.op != 0) && (y.op == 0)))
  {
    return (x.inc > y.r);
  }
  if (((x.op != 0) && (y.op != 0)))
  {
    return (x.inc > y.inc);
  }
}

class query
{
  var x: dynamic;
  var y: dynamic;
  var op: dynamic;
}

var v: dynamic;

var allx: dynamic;

var ally: dynamic;

func add(x: dynamic, y: dynamic)
{
  var i: dynamic;
  {
    i = x;
    while (i)
    {
      s[i] += y;
      i -= ((i & ((-i))));
    }
  }
}

func qry(x: dynamic)
{
  var i: dynamic;
  var sss = 0;
  {
    i = x;
    while ((i <= 800000))
    {
      sss += s[i];
      i += ((i & ((-i))));
    }
  }
  return sss;
}

func solve(v: dynamic, l: dynamic, r: dynamic)
{
  if (v.empty())
  {
    return;
  }
  if ((l >= r))
  {
    return;
  }
  var mid = (((l + r)) / 2);
  var vl: dynamic;
  var vr: dynamic;
  {
    typeof((v).begin()) = (v).begin();
    while ((it != (v).end()))
    {
      if ((it->x <= mid))
      {
        vl.push_back((*it));
        if ((it->op == 0))
        {
          add(it->y, 1);
        }
      } else
      {
        vr.push_back((*it));
        if (it->op)
        {
          ans[it->op] += qry(it->y);
        }
      }
      it += 1;
    }
  }
  {
    typeof((v).begin()) = (v).begin();
    while ((it != (v).end()))
    {
      if ((it->x <= mid))
      {
        if ((it->op == 0))
        {
          add(it->y, -1);
        }
      }
      it += 1;
    }
  }
  solve(vl, l, mid);
  solve(vr, (mid + 1), r);
}

func main()
{
  read(n);
  read(m);
  {
    ((i)) = (1);
    while ((((i)) <= ((n))))
    {
      read(a[i].l);
      ((i)) += 1;
    }
  }
  {
    ((i)) = (1);
    while ((((i)) <= ((n))))
    {
      read(a[i].r);
      ((i)) += 1;
    }
  }
  {
    ((i)) = (1);
    while ((((i)) <= ((n))))
    {
      read(a[i].lys);
      ((i)) += 1;
    }
  }
  {
    ((i)) = (1);
    while ((((i)) <= ((m))))
    {
      read(a[(i + n)].inc);
      ((i)) += 1;
    }
  }
  {
    ((i)) = (1);
    while ((((i)) <= ((m))))
    {
      read(a[(i + n)].pref);
      ((i)) += 1;
    }
  }
  {
    ((i)) = (1);
    while ((((i)) <= ((m))))
    {
      a[(i + n)].op = i;
      ((i)) += 1;
    }
  }
  stable_sort((a + 1), (((a + n) + m) + 1), cmp);
  {
    ((i)) = (1);
    while ((((i)) <= (((n + m)))))
    {
      if (a[i].op)
      {
        v.push_back([(((a[i].pref + a[i].inc) + 1) + 1), ((a[i].pref - a[i].inc) + 1000000001), a[i].op]);
      } else
      {
        v.push_back([((a[i].lys + a[i].l) + 1), ((a[i].lys - a[i].l) + 1000000001), 0]);
      }
      ((i)) += 1;
    }
  }
  {
    typeof((v).begin()) = (v).begin();
    while ((it != (v).end()))
    {
      allx.push_back(it->x);
      ally.push_back(it->y);
      it += 1;
    }
  }
  stable_sort((allx).begin(), (allx).end());
  allx.resize((unique((allx).begin(), (allx).end()) - allx.begin()));
  stable_sort((ally).begin(), (ally).end());
  ally.resize((unique((ally).begin(), (ally).end()) - ally.begin()));
  {
    typeof((v).begin()) = (v).begin();
    while ((it != (v).end()))
    {
      it->x = (upper_bound((allx).begin(), (allx).end(), it->x) - allx.begin());
      it->y = (upper_bound((ally).begin(), (ally).end(), it->y) - ally.begin());
      it += 1;
    }
  }
  solve(v, 1, allx.size());
  {
    ((i)) = (1);
    while ((((i)) <= ((m))))
    {
      printf("%d ", ans[i]);
      ((i)) += 1;
    }
  }
  return 0;
}
