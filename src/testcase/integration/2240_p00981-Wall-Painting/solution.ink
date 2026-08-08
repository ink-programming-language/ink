// Translated from solution.cpp.

var N = cpp_expression("#inclu");

var int_cpp = dynamic;

var Ans: dynamic;

var mx = cpp_array((N << 2), 3, 3);

var X: dynamic;

var Y: dynamic;

var m: dynamic;

var n: dynamic;

var num = cpp_array(N);

var cnt: dynamic;

class Data
{
  var c: dynamic;
  var l: dynamic;
  var r: dynamic;
}

var d = cpp_array(N);

func cmpl(a: dynamic, b: dynamic)
{
  return (a.l < b.l);
}

func pushup(rt: dynamic, c: dynamic)
{
  {
    var i = 0;
    while ((i < 3))
    {
      mx[c][i][rt] = max(mx[c][i][(rt << 1)], mx[c][i][((rt << 1) | 1)]);
      i += 1;
    }
  }
}

func build(rt: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    {
      var i = 0;
      while ((i < 3))
      {
        mx[i][0][rt] = 0;
        mx[i][1][rt] = (X * ((-1 - num[l])));
        mx[i][2][rt] = ((((X + X) + Y)) * ((-1 - num[l])));
        i += 1;
      }
    }
    return;
  }
  var mid = (((l + r)) >> 1);
  build((rt << 1), l, mid);
  build(((rt << 1) | 1), (mid + 1), r);
  pushup(rt, 0);
  pushup(rt, 1);
  pushup(rt, 2);
}

func update(rt: dynamic, l: dynamic, r: dynamic, x: dynamic, y: dynamic, c: dynamic)
{
  if ((l == r))
  {
    mx[c][0][rt] = max(mx[c][0][rt], y);
    mx[c][1][rt] = (mx[c][0][rt] + (X * ((-1 - num[l]))));
    mx[c][2][rt] = (mx[c][0][rt] + ((((X + X) + Y)) * ((-1 - num[l]))));
    return;
  }
  var mid = (((l + r)) >> 1);
  if ((x <= mid))
  {
    update((rt << 1), l, mid, x, y, c);
  } else
  {
    update(((rt << 1) | 1), (mid + 1), r, x, y, c);
  }
  pushup(rt, c);
}

func query(rt: dynamic, l: dynamic, r: dynamic, x: dynamic, y: dynamic, c: dynamic, d: dynamic)
{
  if (((x <= l) && (r <= y)))
  {
    return mx[c][d][rt];
  }
  var mid = (((l + r)) >> 1);
  if ((y <= mid))
  {
    return query((rt << 1), l, mid, x, y, c, d);
  }
  if ((x > mid))
  {
    return query(((rt << 1) | 1), (mid + 1), r, x, y, c, d);
  }
  return max(query((rt << 1), l, mid, x, y, c, d), query(((rt << 1) | 1), (mid + 1), r, x, y, c, d));
}

func main()
{
  scanf("%lld%lld%lld%lld", (&n), (&m), (&X), (&Y));
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%lld%lld%lld", (&d[i].c), (&d[i].l), (&d[i].r));
      d[i].c -= 1;
      num[cpp_update(cnt, "++")] = d[i].l;
      num[cpp_update(cnt, "++")] = d[i].r;
      i += 1;
    }
  }
  sort((num + 1), ((num + cnt) + 1));
  cnt = ((unique((num + 1), ((num + cnt) + 1)) - num) - 1);
  sort((d + 1), ((d + m) + 1), cmpl);
  {
    var i = 1;
    while ((i <= m))
    {
      d[i].l = (lower_bound((num + 1), ((num + cnt) + 1), d[i].l) - num);
      d[i].r = (lower_bound((num + 1), ((num + cnt) + 1), d[i].r) - num);
      i += 1;
    }
  }
  build(1, 0, cnt);
  {
    var i = 1;
    while ((i <= m))
    {
      var ans = LLONG_MIN;
      {
        var j = 0;
        while ((j < 3))
        {
          ans = max(ans, query(1, 0, cnt, 0, (d[i].l - 1), j, 0));
          if ((d[i].l < d[i].r))
          {
            if ((j == d[i].c))
            {
              ans = max(ans, (query(1, 0, cnt, d[i].l, (d[i].r - 1), j, 1) + (num[d[i].l] * X)));
            } else
            {
              ans = max(ans, (query(1, 0, cnt, d[i].l, (d[i].r - 1), j, 2) + (num[d[i].l] * (((X + X) + Y)))));
            }
          }
          j += 1;
        }
      }
      ans += ((((num[d[i].r] - num[d[i].l]) + 1)) * X);
      update(1, 0, cnt, d[i].r, ans, d[i].c);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      Ans = max(Ans, query(1, 0, cnt, 0, cnt, i, 0));
      i += 1;
    }
  }
  printf("%lld\n", Ans);
  return 0;
}
