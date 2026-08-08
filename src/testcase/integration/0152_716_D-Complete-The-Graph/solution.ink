// Translated from solution.cpp.

func break_point()
{
  var c: dynamic;
  while (((cpp_assign(c, "=", getchar())) != cpp_char("\n")))
  {
  }
  return 0;
}

func read_integer(r: dynamic)
{
  var sign = 0;
  r = 0;
  var c: dynamic;
  while (1)
  {
    c = getchar();
    if ((c == cpp_char("-")))
    {
      sign = 1;
      break;
    }
    if (((c != cpp_char(" ")) && (c != cpp_char("\n"))))
    {
      r = (c - cpp_char("0"));
      break;
    }
  }
  while (1)
  {
    c = getchar();
    if (((c == cpp_char(" ")) || (c == cpp_char("\n"))))
    {
      break;
    }
    r = ((r * 10) + ((c - cpp_char("0"))));
  }
  if (sign)
  {
    r = (-r);
  }
}

func binpowmod(a: dynamic, b: dynamic, mod: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  var c = binpowmod(a, (b >> 1), mod);
  return (((((((c * c)) % mod)) * (if ((b & 1)) a else 1))) % mod);
}

func binpow(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  var c = binpow(a, (b >> 1));
  return ((c * c) * (if ((b & 1)) a else 1));
}

func getbit(x: dynamic, b: dynamic)
{
  return (((x >> b)) & 1);
}

func setbit(x: dynamic, b: dynamic)
{
  return (x | ((1 << b)));
}

func setbit(x: dynamic, b: dynamic)
{
  x = setbit(x, b);
}

func setbit(x: dynamic, b: dynamic)
{
  return (x | ((1 << b)));
}

func setbit(x: dynamic, b: dynamic)
{
  x = setbit(x, b);
}

func unsetbit(x: dynamic, b: dynamic)
{
  return (x & ((INT_MAX - ((1 << b)))));
}

func unsetbit(x: dynamic, b: dynamic)
{
  x = unsetbit(x, b);
}

func countbit(x: dynamic)
{
  x = (x - ((((x >> 1)) & 0x55555555)));
  x = (((x & 0x33333333)) + ((((x >> 2)) & 0x33333333)));
  return ((((((x + ((x >> 4))) & 0xF0F0F0F)) * 0x1010101)) >> 24);
}

func countbit(x: dynamic)
{
  return (countbit(int_cpp((x & INT_MAX))) + countbit((int_cpp((x >> 32)) & INT_MAX)));
}

func printbit(x: dynamic, len: dynamic)
{
  {
    var i = (len - 1);
    while ((i >= 0))
    {
      printf("%d", getbit(x, i));
      i -= 1;
    }
  }
}

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func operator_shift_left(stream: dynamic, p: dynamic)
{
  (((((stream << "{") << p.first) << ",") << p.second) << "}");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  (stream << "[");
  {
    var itr = v.begin();
    while ((itr != v.end()))
    {
      ((stream << (*itr)) << " ");
      itr += 1;
    }
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  (stream << "[");
  {
    var itr = v.begin();
    while ((itr != v.end()))
    {
      ((stream << (*itr)) << " ");
      itr += 1;
    }
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  (stream << "[");
  {
    var itr = v.begin();
    while ((itr != v.end()))
    {
      ((stream << (*itr)) << " ");
      itr += 1;
    }
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  var st = v;
  (stream << "[");
  while ((!st.empty()))
  {
    ((stream << st.top()) << " ");
    st.pop();
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  var q = v;
  (stream << "[");
  while ((!q.empty()))
  {
    ((stream << q.top()) << " ");
    q.pop();
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  var q = v;
  (stream << "[");
  while ((!q.empty()))
  {
    ((stream << q.front()) << " ");
    q.pop();
  }
  (stream << "]");
  return stream;
}

func operator_shift_left(stream: dynamic, v: dynamic)
{
  var q = v;
  (stream << "[");
  while ((!q.empty()))
  {
    ((stream << q.front()) << " ");
    q.pop_front();
  }
  (stream << "]");
  return stream;
}

func main()
{
  srand(time(null));
  run();
  return 0;
}

var mod = (1e9 + 7);

var N = 1003;

class Edge
{
  var a: dynamic;
  var b: dynamic;
  var cost: dynamic;
  func Edge(a: dynamic = 0, b: dynamic = 0, cost: dynamic = 0)
  {
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
      this->cost = cpp_construct(cost);
    }
  func to(from_cpp: dynamic)
  {
      return if ((from_cpp == a)) b else a;
    }
}

var g = cpp_array(N);

var e: dynamic;

var p = cpp_construct(N, -1);

var u = cpp_construct((N * 100), false);

func dejkstra(s: dynamic, t: dynamic, f: dynamic = false)
{
  fill(d.begin(), d.end(), LLONG_MAX);
  var st: dynamic;
  d[s] = 0;
  st.insert([0, s]);
  while ((!st.empty()))
  {
    var v = ((*st.begin())).second;
    st.erase(st.begin());
    {
      var i = 0;
      while ((i < (cpp_cast(g[v].size()))))
      {
        var edg = e[g[v][i]];
        if (((edg.cost == 0) && f))
        {
          i += 1;
          continue;
        }
        var cost = max(1, edg.cost);
        if ((d[edg.to(v)] > (d[v] + cost)))
        {
          st.erase([d[edg.to(v)], edg.to(v)]);
          d[edg.to(v)] = (d[v] + cost);
          p[edg.to(v)] = g[v][i];
          st.insert([d[edg.to(v)], edg.to(v)]);
        }
        i += 1;
      }
    }
  }
  if (0) (((((cout << "d[t]") << " = ") << (d[t])) << "\n")) else cout;
  return d[t];
}

func run()
{
  var n: dynamic;
  var m: dynamic;
  var L: dynamic;
  var s: dynamic;
  var t: dynamic;
  scanf("%d%d", (&n), (&m));
  scanf("%d%d%d", (&L), (&s), (&t));
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d%d%d", (&a), (&b), (&c));
      g[a].push_back((cpp_cast(e.size())));
      g[b].push_back((cpp_cast(e.size())));
      e.push_back(Edge(a, b, c));
      i += 1;
    }
  }
  if ((dejkstra(s, t, true) < L))
  {
    printf("NO\n");
    return;
  }
  while ((dejkstra(s, t) <= L))
  {
    var v = t;
    var vct: dynamic;
    while ((v != s))
    {
      var id = p[v];
      if ((e[id].cost == 0))
      {
        vct.push_back(id);
        u[id] = 1;
      }
      v = e[id].to(v);
    }
    if (vct.empty())
    {
      break;
    }
    var need = (L - d[t]);
    e[vct.back()].cost = (1 + need);
  }
  if ((dejkstra(s, t) != L))
  {
    printf("NO\n");
    return;
  }
  {
    var i = 0;
    while ((i < m))
    {
      if ((e[i].cost == 0))
      {
        e[i].cost = if (u[i]) 1 else (1 << 50);
      }
      i += 1;
    }
  }
  printf("YES\n");
  {
    var i = 0;
    while ((i < m))
    {
      printf("%d %d %lld\n", e[i].a, e[i].b, e[i].cost);
      i += 1;
    }
  }
}
