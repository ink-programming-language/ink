// Translated from solution.cpp.

var n: dynamic;

var to = cpp_array(200050);

var nxt = cpp_array(200050);

var fir = cpp_array(100050);

var ans: dynamic;

var t1: dynamic;

var t2: dynamic;

var tot: dynamic;

var siz = cpp_array(100050);

var son = cpp_array(100050);

var mx = cpp_array(100050);

var rt: dynamic;

var rt2: dynamic;

var dep = cpp_array(100040);

func ade(x: dynamic, y: dynamic)
{
  to[cpp_update(tot, "++")] = y;
  nxt[tot] = fir[x];
  fir[x] = tot;
}

func find(x: dynamic, fa: dynamic)
{
  siz[x] = 1;
  mx[x] = 0;
  dep[x] = (dep[fa] + 1);
  var bb = 1;
  {
    var k = fir[x];
    while (k)
    {
      if ((to[k] == fa))
      {
        k = nxt[k];
        continue;
      }
      find(to[k], x);
      mx[x] = max(mx[x], siz[to[k]]);
      son[x] = son[to[k]];
      siz[x] += siz[to[k]];
      bb = 0;
      k = nxt[k];
    }
  }
  if (bb)
  {
    son[x] = x;
  }
  mx[x] = max(mx[x], (n - siz[x]));
  if ((mx[x] < mx[rt]))
  {
    rt = x;
    rt2 = 0;
  } else if ((mx[x] == mx[rt]))
  {
    rt2 = x;
  }
}

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    mx[0] = n;
    rt = cpp_assign(rt2, "=", 0);
    tot = 0;
    memset(fir, 0, cpp_sizeof((fir)));
    {
      var i = 1;
      while ((i <= (n - 1)))
      {
        var x: dynamic;
        var y: dynamic;
        read(x, y);
        ade(x, y);
        ade(y, x);
        i += 1;
      }
    }
    find(1, 0);
    if ((rt2 == 0))
    {
      write(1, " ", to[fir[1]], "\n", 1, " ", to[fir[1]], "\n");
    } else
    {
      if ((dep[rt] < dep[rt2]))
      {
        swap(rt, rt2);
      }
      write(son[rt], " ", to[fir[son[rt]]], "\n", rt2, " ", son[rt], "\n");
    }
  }
  return 0;
}
