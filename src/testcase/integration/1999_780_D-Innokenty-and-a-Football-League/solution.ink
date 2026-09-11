// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

class node
{
  var fi: dynamic = cpp_uninitialized();
  var se: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var x: dynamic = cpp_array(1009);

var n: dynamic = cpp_uninitialized();

var cntfi: dynamic = cpp_uninitialized();

var cntse: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (cntfi[a.fi] > cntfi[b.fi]);
}

func cmp_cnt(a: dynamic, b: dynamic) -> dynamic
{
  return (cnt[a.fi] > cnt[b.fi]);
}

func cmp_id(a: dynamic, b: dynamic) -> dynamic
{
  return (a.id < b.id);
}

func f(k: dynamic) -> dynamic
{
  sort((x + k), (x + n), cmp_cnt);
  if ((!cnt[x[k].fi]))
  {
    return k;
  }
  {
    while ((k < n))
    {
      if (cnt[x[k].fi])
      {
        if (cnt[x[k].se])
        {
          return -1;
        }
        x[k].ans = x[k].se;
        cnt[x[k].ans] = 1;
      } else
      {
        break;
      }
      k += 1;
    }
  }
  return f(k);
}

func solve() -> dynamic
{
  var i: dynamic = 0;
  {
    while ((i < n))
    {
      if ((cntfi[x[i].fi] > 1))
      {
        if ((cntse[x[i].se] == 1))
        {
          return false;
        } else
        {
          x[i].ans = x[i].se;
          cntse[x[i].ans] = 1;
          cnt[x[i].ans] = 1;
        }
      } else
      {
        break;
      }
      i += 1;
    }
  }
  i = f(i);
  if ((i == -1))
  {
    return false;
  }
  {
    while ((i < n))
    {
      x[i].ans = x[i].fi;
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  while ((~scanf("%d", (&n))))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(x[i].fi, x[i].se);
        x[i].id = i;
        x[i].se = (x[i].fi.substr(0, 2) + x[i].se[0]);
        x[i].fi = x[i].fi.substr(0, 3);
        cntfi[x[i].fi] += 1;
        i += 1;
      }
    }
    sort(x, (x + n), cmp);
    var flag: dynamic = solve();
    printf("%s\n",  (flag) ? "YES" : "NO");
    if (flag)
    {
      sort(x, (x + n), cmp_id);
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          write(x[i].ans, "\n");
          i += 1;
        }
      }
    }
  }
  return 0;
}
