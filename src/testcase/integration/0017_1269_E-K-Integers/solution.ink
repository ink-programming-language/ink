// Translated from solution.cpp.

var N: dynamic = (1e6 + 10);

var mod: dynamic = (1e9 + 7);

var head: dynamic = cpp_array(N);

var dis: dynamic = cpp_array(N);

var ecnt: dynamic = cpp_uninitialized();

var fa: dynamic = cpp_array(N);

var cat: dynamic = cpp_array(2005, 2005);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (((a % b) == 0)) ? b : gcd(b, (a % b));
}

func qpow(base: dynamic, n: dynamic) -> dynamic
{
  var ans: dynamic = 1;
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
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var nd: dynamic = cpp_array(N);

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

class EDGE
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var nxt: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(N);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.w < b.w);
}

func add_edge(u: dynamic, v: dynamic, w: dynamic) -> dynamic
{
  e[ecnt].u = u;
  e[ecnt].v = v;
  e[ecnt].w = w;
  e[ecnt].nxt = head[u];
  head[u] = cpp_update(ecnt, "++");
}

func fd(x: dynamic) -> dynamic
{
  return  ((-1 == fa[x])) ? x : cpp_assign(fa[x], "=", fd(fa[x]));
}

var c: dynamic = cpp_array(N);

func lowbit(x: dynamic) -> dynamic
{
  return (x & ((-x)));
}

func add(c: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  while ((x <= n))
  {
    c[x] += y;
    x += lowbit(x);
  }
}

func getsum(c: dynamic, x: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while ((x > 0))
  {
    res += c[x];
    x -= lowbit(x);
  }
  return res;
}

var vis: dynamic = cpp_array(N);

var p: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

var C: dynamic = cpp_array(55, 55);

var dp: dynamic = cpp_array(150, 150);

var posar: dynamic = cpp_array(N);

var smar: dynamic = cpp_array(N);

class H
{
  var p: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
}

var he: dynamic = cpp_array(N);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a > b);
}

var q: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(30);

var mp: dynamic = cpp_uninitialized();

var ar: dynamic = cpp_array(N);

var br: dynamic = cpp_array(N);

var vv: dynamic = cpp_array(10);

func main() -> dynamic
{
  {
    read(n);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        scanf("%d", (&ar[i]));
        p[ar[i]] = i;
        i += 1;
      }
    }
    var inv: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        inv += ((i - 1) - getsum(smar, p[i]));
        add(smar, p[i], 1);
        add(posar, p[i], p[i]);
        var l: dynamic = 1;
        var r: dynamic = n;
        var mid: dynamic = cpp_uninitialized();
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
        var pre_cnt_sum: dynamic = getsum(smar, mid);
        var pre_pos_sum: dynamic = getsum(posar, mid);
        var mov: dynamic = (((pre_cnt_sum * mid) - pre_pos_sum) - ((pre_cnt_sum * ((pre_cnt_sum - 1))) / 2));
        var aft_cnt_sum: dynamic = (i - pre_cnt_sum);
        mov += (((getsum(posar, n) - pre_pos_sum) - (aft_cnt_sum * mid)) - ((aft_cnt_sum * ((aft_cnt_sum + 1))) / 2));
        write((inv + mov), " \n"[(i == n)]);
        i += 1;
      }
    }
  }
  return 0;
}
