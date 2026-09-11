// Translated from solution.cpp.

var INF: dynamic = (1e9 + 7);

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
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
    x = (((((x << 1)) + ((x << 3))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var n: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(100005);

var a: dynamic = cpp_array(100005);

var pre: dynamic = cpp_array(100005);

var nxt: dynamic = cpp_array(100005);

var x: dynamic = cpp_array(100005);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (p[a] < p[b]);
}

var st: dynamic = cpp_uninitialized();

func dis(i: dynamic, j: dynamic) -> dynamic
{
  if ((i == j))
  {
    return INF;
  }
  var d: dynamic = ((((p[j] - p[i]) + L)) % L);
  if ((i > j))
  {
    d = (((d + a[j])) % L);
  }
  if ((d <= a[i]))
  {
    return 1;
  }
  if ((a[i] <= a[j]))
  {
    return INF;
  }
  return (((((d - a[j]) - 1)) / ((a[i] - a[j]))) + 1);
}

func main() -> dynamic
{
  n = read();
  L = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i] = read();
      a[i] = read();
      x[i] = i;
      i += 1;
    }
  }
  sort((x + 1), ((x + 1) + n), cmp);
  pre[x[1]] = x[n];
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((i > 1))
      {
        pre[x[i]] = x[(i - 1)];
      }
      nxt[pre[x[i]]] = x[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      st.insert(make_pair(dis(i, nxt[i]), i));
      i += 1;
    }
  }
  var it: dynamic = cpp_uninitialized();
  while (st.size())
  {
    it = st.begin();
    var i: dynamic = it->second;
    var d: dynamic = it->first;
    if ((d == INF))
    {
      break;
    }
    st.erase(it);
    st.erase(make_pair(dis(nxt[i], nxt[nxt[i]]), nxt[i]));
    st.erase(make_pair(dis(pre[i], i), pre[i]));
    p[i] = (((((p[i] + d) - 1)) % L) + 1);
    a[i] -= 1;
    nxt[i] = nxt[nxt[i]];
    pre[nxt[i]] = i;
    st.insert(make_pair(dis(pre[i], i), pre[i]));
    st.insert(make_pair(dis(i, nxt[i]), i));
  }
  printf("%d\n", st.size());
  while (st.size())
  {
    it = st.begin();
    printf("%d ", it->second);
    st.erase(it);
  }
  return 0;
}
