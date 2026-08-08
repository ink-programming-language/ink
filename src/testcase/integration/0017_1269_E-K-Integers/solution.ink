// Translated from solution.cpp.

var N = (1e6 + 10);

var mod = (1e9 + 7);

var head = cpp_array(N);

var dis = cpp_array(N);

var ecnt: dynamic;

var fa = cpp_array(N);

var cat = cpp_array(2005, 2005);

func gcd(a: dynamic, b: dynamic)
{
  return if (((a % b) == 0)) b else gcd(b, (a % b));
}

func qpow(base: dynamic, n: dynamic)
{
  var ans = 1;
  while (n)
  {
    if ((n & 1))
    {
      ans = (((ans * base)) % mod);
    }
    n >>= 1;
    base = ((base * base) % mod);
  }
  return ans;
}

class Node
{
  var x: dynamic;
  var y: dynamic;
}

var nd = cpp_array(N);

var m: dynamic;

var n: dynamic;

var x: dynamic;

var k: dynamic;

var y: dynamic;

class EDGE
{
  var u: dynamic;
  var v: dynamic;
  var nxt: dynamic;
  var w: dynamic;
}

var e = cpp_array(N);

func cmp(a: dynamic, b: dynamic)
{
  return (a.w < b.w);
}

func add_edge(u: dynamic, v: dynamic, w: dynamic)
{
  e[ecnt].u = u;
  e[ecnt].v = v;
  e[ecnt].w = w;
  e[ecnt].nxt = head[u];
  head[u] = cpp_update(ecnt, "++");
}

func fd(x: dynamic)
{
  return if ((-1 == fa[x])) x else cpp_assign(fa[x], "=", fd(fa[x]));
}

var c = cpp_array(N);

func lowbit(x: dynamic)
{
  return (x & ((-x)));
}

func add(c: dynamic, x: dynamic, y: dynamic)
{
  while ((x <= n))
  {
    c[x] += y;
    x += lowbit(x);
  }
}

func getsum(c: dynamic, x: dynamic)
{
  var res = 0;
  while ((x > 0))
  {
    res += c[x];
    x -= lowbit(x);
  }
  return res;
}

var vis = cpp_array(N);

var p = cpp_array(N);

var ans: dynamic;

var C = cpp_array(55, 55);

var dp = cpp_array(150, 150);

var posar = cpp_array(N);

var smar = cpp_array(N);

class H
{
  var p: dynamic;
  var s: dynamic;
}

var he = cpp_array(N);

func cmp(a: dynamic, b: dynamic)
{
  return (a > b);
}

var q: dynamic;

var v = cpp_array(30);

var mp: dynamic;

var ar = cpp_array(N);

var br = cpp_array(N);

var vv = cpp_array(10);

func main()
{
  {
    read(n);
    {
      var i = 1;
      while ((i <= n))
      {
        scanf("%d", (&ar[i]));
        p[ar[i]] = i;
        i += 1;
      }
    }
    var inv = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        inv += ((i - 1) - getsum(smar, p[i]));
        add(smar, p[i], 1);
        add(posar, p[i], p[i]);
        var l = 1;
        var r = n;
        var mid: dynamic;
        while ((l < r))
        {
          mid = (((1 + l) + r) >> 1);
          if (((getsum(smar, (mid - 1)) * 2) <= i))
          {
            l = mid;
          } else
          {
            r = (mid - 1);
          }
        }
        mid = l;
        var pre_cnt_sum = getsum(smar, mid);
        var pre_pos_sum = getsum(posar, mid);
        var mov = (((pre_cnt_sum * mid) - pre_pos_sum) - ((pre_cnt_sum * ((pre_cnt_sum - 1))) / 2));
        var aft_cnt_sum = (i - pre_cnt_sum);
        mov += (((getsum(posar, n) - pre_pos_sum) - (aft_cnt_sum * mid)) - ((aft_cnt_sum * ((aft_cnt_sum + 1))) / 2));
        write((inv + mov), " \n"[(i == n)]);
        i += 1;
      }
    }
  }
  return 0;
}
