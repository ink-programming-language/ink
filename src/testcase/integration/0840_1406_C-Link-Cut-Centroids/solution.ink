// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var to: dynamic = cpp_array(200050);

var nxt: dynamic = cpp_array(200050);

var fir: dynamic = cpp_array(100050);

var ans: dynamic = cpp_uninitialized();

var t1: dynamic = cpp_uninitialized();

var t2: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var siz: dynamic = cpp_array(100050);

var son: dynamic = cpp_array(100050);

var mx: dynamic = cpp_array(100050);

var rt: dynamic = cpp_uninitialized();

var rt2: dynamic = cpp_uninitialized();

var dep: dynamic = cpp_array(100040);

func ade(x: dynamic, y: dynamic) -> dynamic
{
  to[cpp_update(tot, "++")] = y;
  nxt[tot] = fir[x];
  fir[x] = tot;
}

func find(x: dynamic, fa: dynamic) -> dynamic
{
  siz[x] = 1;
  mx[x] = 0;
  dep[x] = (dep[fa] + 1);
  var bb: dynamic = 1;
  {
    var k: dynamic = fir[x];
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

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    mx[0] = n;
    rt = cpp_assign(rt2, "=", 0);
    tot = 0;
    memset(fir, 0, cpp_sizeof((fir)));
    {
      var i: dynamic = 1;
      while ((i <= (n - 1)))
      {
        var x: dynamic = cpp_uninitialized();
        var y: dynamic = cpp_uninitialized();
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
