// Translated from solution.cpp.

var maxN: dynamic = (1e5 + 5);

var maxM: dynamic = (1e3 + 5);

var INF: dynamic = 1e18;

var MOD: dynamic = (1e9 + 7);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((!b)) ? a : gcd(b, (a % b));
}

func sq(x: dynamic) -> dynamic
{
  return (((x * x)) % MOD);
}

func modP(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((!b)) ? 1 : (((sq(modP(a, (b / 2))) * ( ((b % 2)) ? a : 1))) % MOD));
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxN);

var b: dynamic = cpp_array(maxN);

var lazy: dynamic = cpp_array((4 * maxN));

var st: dynamic = cpp_array(maxN);

var fn: dynamic = cpp_array(maxN);

var T: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(maxN);

var prime: dynamic = cpp_uninitialized();

var seg: dynamic = cpp_array((4 * maxN));

var null_cpp: dynamic = cpp_uninitialized();

func DFS(v: dynamic, p: dynamic = -1) -> dynamic
{
  st[v] = cpp_update(T, "++");
  for (var u: dynamic in G[v])
  {
    if ((u == p))
    {
      continue;
    }
    DFS(u, v);
  }
  fn[v] = T;
  return;
}

func build(id: dynamic = 1, s: dynamic = 0, e: dynamic = n) -> dynamic
{
  if (((e - s) <= 1))
  {
    seg[id].set((a[s] % m));
    return;
  }
  var md: dynamic = (((s + e)) / 2);
  build((2 * id), s, md);
  build(((2 * id) + 1), md, e);
  seg[id] = (seg[(2 * id)] | seg[((2 * id) + 1)]);
  return;
}

func apply(id: dynamic, x: dynamic) -> dynamic
{
  seg[id] = (((seg[id] << x)) | ((seg[id] >> ((m - x)))));
  lazy[id] = (((lazy[id] + x)) % m);
  return;
}

func shift(id: dynamic) -> dynamic
{
  if ((lazy[id] == 0))
  {
    return;
  }
  apply((2 * id), (lazy[id] % m));
  apply(((2 * id) + 1), (lazy[id] % m));
  lazy[id] = 0;
  return;
}

func update(l: dynamic, r: dynamic, x: dynamic, id: dynamic = 1, s: dynamic = 0, e: dynamic = n) -> dynamic
{
  if (((l <= s) && (e <= r)))
  {
    apply(id, (x % m));
    return;
  }
  if (((l >= e) || (s >= r)))
  {
    return;
  }
  var md: dynamic = (((s + e)) / 2);
  shift(id);
  update(l, r, x, (2 * id), s, md);
  update(l, r, x, ((2 * id) + 1), md, e);
  seg[id] = (seg[(2 * id)] | seg[((2 * id) + 1)]);
  return;
}

func get(l: dynamic, r: dynamic, id: dynamic = 1, s: dynamic = 0, e: dynamic = n) -> dynamic
{
  if (((l <= s) && (e <= r)))
  {
    return seg[id];
  }
  if (((l >= e) || (s >= r)))
  {
    return null_cpp;
  }
  var md: dynamic = (((e + s)) / 2);
  shift(id);
  return ((get(l, r, (2 * id), s, md) | get(l, r, ((2 * id) + 1), md, e)));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  prime.set();
  prime.set(0, 0);
  prime.set(1, 0);
  {
    var i: dynamic = 2;
    while ((i < m))
    {
      if ((prime[i] == 1))
      {
        {
          var j: dynamic = (2 * i);
          while ((j < m))
          {
            prime.set(j, 0);
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = m;
    while ((i < maxM))
    {
      prime.set(i, 0);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      G[u].push_back(v);
      G[v].push_back(u);
      i += 1;
    }
  }
  DFS(0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a[st[i]] = b[i];
      i += 1;
    }
  }
  build();
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    var t: dynamic = cpp_uninitialized();
    read(t);
    if ((t == 1))
    {
      var v: dynamic = cpp_uninitialized();
      var x: dynamic = cpp_uninitialized();
      read(v, x);
      v -= 1;
      update(st[v], fn[v], x);
    } else
    {
      var v: dynamic = cpp_uninitialized();
      read(v);
      v -= 1;
      write(((prime & get(st[v], fn[v]))).count(), "\n");
    }
  }
  return 0;
}
