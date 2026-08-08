// Translated from solution.cpp.

var INF = (1e9 + 7);

func read()
{
  var x = 0;
  var f = 1;
  var ch = getchar();
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

var n: dynamic;

var L: dynamic;

var p = cpp_array(100005);

var a = cpp_array(100005);

var pre = cpp_array(100005);

var nxt = cpp_array(100005);

var x = cpp_array(100005);

func cmp(a: dynamic, b: dynamic)
{
  return (p[a] < p[b]);
}

var st: dynamic;

func dis(i: dynamic, j: dynamic)
{
  if ((i == j))
  {
    return INF;
  }
  var d = ((((p[j] - p[i]) + L)) % L);
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

func main()
{
  n = read();
  L = read();
  {
    var i = 1;
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
    var i = 1;
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
    var i = 1;
    while ((i <= n))
    {
      st.insert(make_pair(dis(i, nxt[i]), i));
      i += 1;
    }
  }
  var it: dynamic;
  while (st.size())
  {
    it = st.begin();
    var i = it->second;
    var d = it->first;
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
