// Translated from solution.cpp.

var maxn: dynamic = (2e6 + 100);

var inf: dynamic = 0x3f3f3f3f;

var iinf: dynamic = (1 << 30);

var linf: dynamic = 2e18;

var mod: dynamic = 998244353;

var eps: dynamic = 1e-7;

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", min(a, b));
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_assign(a, "=", max(a, b));
}

func read() -> dynamic
{
  var f: dynamic = 1;
  var a: dynamic = 0;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    a = (((((a << 3)) + ((a << 1))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (a * f);
}

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var dif_sum: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(maxn);

var sum: dynamic = cpp_array(maxn);

var dif: dynamic = cpp_uninitialized();

var col: dynamic = cpp_uninitialized();

var near: dynamic = cpp_array(maxn);

class node
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
  var tag: dynamic = cpp_uninitialized();
}

var tr: dynamic = cpp_array(maxn);

func build(tot: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  tr[tot].l = l;
  tr[tot].r = r;
  tr[tot].val = cpp_assign(tr[tot].tag, "=", 0);
  if ((l == r))
  {
    return;
  }
  build(((tot << 1)), l, ((((tr[tot].l + tr[tot].r)) >> 1)));
  build((((tot << 1) | 1)), (((((tr[tot].l + tr[tot].r)) >> 1)) + 1), r);
}

func stag(tot: dynamic) -> dynamic
{
  tr[tot].val = ((tr[tot].r - tr[tot].l) + 1);
  if ((tr[tot].l != tr[tot].r))
  {
    tr[tot].tag = 1;
  }
}

func pushdown(tot: dynamic) -> dynamic
{
  if ((!tr[tot].tag))
  {
    return;
  }
  stag(((tot << 1)));
  stag((((tot << 1) | 1)));
  tr[tot].tag = 0;
}

func maintain(tot: dynamic) -> dynamic
{
  tr[tot].val = (tr[((tot << 1))].val + tr[(((tot << 1) | 1))].val);
}

func modify(tot: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((tr[tot].l >= l) && (tr[tot].r <= r)))
  {
    return cpp_cast((stag(tot)));
  }
  pushdown(tot);
  if ((l <= ((((tr[tot].l + tr[tot].r)) >> 1))))
  {
    modify(((tot << 1)), l, r);
  }
  if ((r > ((((tr[tot].l + tr[tot].r)) >> 1))))
  {
    modify((((tot << 1) | 1)), l, r);
  }
  maintain(tot);
}

func query(tot: dynamic, r: dynamic) -> dynamic
{
  if ((tr[tot].r <= r))
  {
    return tr[tot].val;
  }
  pushdown(tot);
  var ret: dynamic = query(((tot << 1)), r);
  if ((r > ((((tr[tot].l + tr[tot].r)) >> 1))))
  {
    ret += query((((tot << 1) | 1)), r);
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%lld", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%s", (s + 1));
    n = strlen((s + 1));
    build(1, 1, n);
    dif_sum = 0;
    {
      var i: dynamic = (1);
      while ((i <= ((n - 1))))
      {
        if ((s[i] == s[(i + 1)]))
        {
          dif.insert(make_pair(i, (s[i] - cpp_char("a"))));
          sum[(s[i] - cpp_char("a"))] -= 1;
          dif_sum += 1;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = (0);
      while ((i <= (25)))
      {
        if (sum[i])
        {
          col.insert(make_pair(sum[i], i));
        }
        i += 1;
      }
    }
    var last: dynamic = dif.begin();
    if ((last == dif.end()))
    {
      puts("1");
      cpp_goto("goto end;");
    }
    {
      var i: dynamic = cpp_update(dif.begin(), "++");
      while ((i != dif.end()))
      {
        if ((i->second != last->second))
        {
          near[i->second].insert(make_pair((*last), (*i)));
          near[last->second].insert(make_pair((*last), (*i)));
        }
        last = i;
        i += 1;
      }
    }
    ans = (max((((dif_sum + 1)) / 2), (-col.begin()->first)) + 1);
    printf("%lld\n", ans);
    while ((col.size() > 1))
    {
      var b: dynamic = (*col.begin());
      var del: dynamic = (*near[b.second].begin());
      var l: dynamic = ((del.first.first + 1) - query(1, (del.first.first + 1)));
      var r: dynamic = (del.second.first - query(1, del.second.first));
      printf("%lld %lld\n", l, r);
      modify(1, (del.first.first + 1), del.second.first);
      near[del.first.second].erase(near[del.first.second].find(del));
      near[del.second.second].erase(near[del.second.second].find(del));
      var posl: dynamic = dif.find(del.first);
      var posr: dynamic = cpp_update(dif.find(del.second), "++");
      if ((posl != dif.begin()))
      {
        posl -= 1;
        if ((posl->second != del.first.second))
        {
          near[posl->second].erase(near[posl->second].find(make_pair((*posl), del.first)));
          near[del.first.second].erase(near[del.first.second].find(make_pair((*posl), del.first)));
        }
        posl += 1;
      }
      if ((posr != dif.end()))
      {
        if ((del.second.second != posr->second))
        {
          near[del.second.second].erase(near[del.second.second].find(make_pair(del.second, (*posr))));
          near[posr->second].erase(near[posr->second].find(make_pair(del.second, (*posr))));
        }
      }
      if (((posl != dif.begin()) && (posr != dif.end())))
      {
        posl -= 1;
        if ((posl->second != posr->second))
        {
          near[posl->second].insert(make_pair((*posl), (*posr)));
          near[posr->second].insert(make_pair((*posl), (*posr)));
        }
      }
      dif.erase(dif.find(del.first));
      dif.erase(dif.find(del.second));
      col.erase(col.find(make_pair(sum[del.first.second], del.first.second)));
      col.erase(col.find(make_pair(sum[del.second.second], del.second.second)));
      sum[del.first.second] += 1;
      sum[del.second.second] += 1;
      if (sum[del.first.second])
      {
        col.insert(make_pair(sum[del.first.second], del.first.second));
      }
      if (sum[del.second.second])
      {
        col.insert(make_pair(sum[del.second.second], del.second.second));
      }
    }
    if ((col.size() == 1))
    {
      sum[col.begin()->second] = 0;
      col.clear();
      while (dif.size())
      {
        var top: dynamic = dif.begin();
        printf("1 %lld\n", (top->first - query(1, top->first)));
        modify(1, 1, top->first);
        dif.erase(top);
      }
    }
    printf("1 %lld\n", (n - query(1, n)));
  }
  return 0;
}
