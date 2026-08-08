// Translated from solution.cpp.

var mod = (1e9 + 7);

var maxm = (5e3 + 10);

var inf = 0x3f3f3f3f;

var SIZE = (((1 << 21)) + 1);

var ibuf = cpp_array(SIZE);

var iS: dynamic;

var iT: dynamic;

var obuf = cpp_array(SIZE);

var oS = obuf;

var oT = ((oS + SIZE) - 1);

var c: dynamic;

var qu = cpp_array(55);

var f: dynamic;

var qr: dynamic;

func flush()
{
  fwrite(obuf, 1, (oS - obuf), stdout);
  oS = obuf;
}

func putc(x: dynamic)
{
  (*cpp_update(oS, "++")) = x;
  if ((oS == oT))
  {
    flush();
  }
}

func read(x: dynamic)
{
  {
    f = 1;
    c = (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++")));
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      } else if ((c == EOF))
      {
        return 0;
      }
      c = (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++")));
    }
  }
  {
    x = 0;
    while (((c <= cpp_char("9")) && (c >= cpp_char("0"))))
    {
      x = ((x * 10) + ((c & 15)));
      c = (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++")));
    }
  }
  x *= f;
  return 1;
}

func read(x: dynamic)
{
  while (((((cpp_assign(x, "=", (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++"))))) == cpp_char(" ")) || (x == cpp_char("\n"))) || (x == cpp_char("\r"))))
  {
  }
  return (x != EOF);
}

func read(x: dynamic)
{
  while (((((cpp_assign((*x), "=", (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++"))))) == cpp_char("\n")) || ((*x) == cpp_char(" "))) || ((*x) == cpp_char("\r"))))
  {
  }
  if (((*x) == EOF))
  {
    return 0;
  }
  while ((!((((((*x) == cpp_char("\n")) || ((*x) == cpp_char(" "))) || ((*x) == cpp_char("\r"))) || ((*x) == EOF)))))
  {
    (*(cpp_update(x, "++"))) = (if ((iS == iT)) (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))(if ((iS == iT)) EOF else (*cpp_update(iS, "++"))))) else (*cpp_update(iS, "++")));
  }
  (*x) = 0;
  return 1;
}

func read(x: dynamic, y: dynamic...)
{
  return (read(x) && read(cpp_expand(y)));
}

func write(x: dynamic)
{
  if ((!x))
  {
    putc(cpp_char("0"));
  }
  if ((x < 0))
  {
    putc(cpp_char("-"));
    x = (-x);
  }
  while (x)
  {
    qu[cpp_update(qr, "++")] = ((x % 10) + cpp_char("0"));
    x /= 10;
  }
  while (qr)
  {
    putc(qu[cpp_update(qr, "--")]);
  }
  return 0;
}

func write(x: dynamic)
{
  putc(x);
  return 0;
}

func write(x: dynamic)
{
  while ((*x))
  {
    putc((*x));
    x += 1;
  }
  return 0;
}

func write(x: dynamic)
{
  while ((*x))
  {
    putc((*x));
    x += 1;
  }
  return 0;
}

func write(x: dynamic, y: dynamic...)
{
  return (write(x) || write(cpp_expand(y)));
}

class Flusher
{
  func ~Flusher()
  {
      flush();
    }
}

var io_flusher: dynamic;

var eee = cpp_array(maxm);

var s: dynamic;

var t: dynamic;

var dis = cpp_array(maxm);

var h = cpp_array(maxm);

var fnasiofnoas = cpp_array(maxm);

var pree = cpp_array(maxm);

var num = cpp_array(maxm);

class edge
{
  var u: dynamic;
  var v: dynamic;
  var c: dynamic;
  var w: dynamic;
  var rev: dynamic;
  func edge(a: dynamic = -1, b: dynamic = 0, cc: dynamic = 0, d: dynamic = 0, f: dynamic = 0)
  {
      this->rev = cpp_construct(a);
      this->u = cpp_construct(b);
      this->v = cpp_construct(cc);
      this->c = cpp_construct(d);
      this->w = cpp_construct(f);
    }
}

var ed = cpp_array(maxm);

func addedge(u: dynamic, v: dynamic, c: dynamic, w: dynamic)
{
  ed[u].push_back(edge(cpp_cast(ed[v].size()), u, v, c, w));
  ed[v].push_back(edge((cpp_cast(ed[u].size()) - 1), v, u, 0, (-w)));
}

class node
{
  var id: dynamic;
  var val: dynamic;
  func node(a: dynamic = 0, b: dynamic = 0)
  {
      this->id = cpp_construct(a);
      this->val = cpp_construct(b);
    }
}

var pq: dynamic;

func costflow()
{
  var res = 0;
  memset(h, 0, cpp_sizeof((h)));
  var tot = inf;
  while ((tot > 0))
  {
    memset(dis, 0x3f, cpp_sizeof((dis)));
    while ((!pq.empty()))
    {
      pq.pop();
    }
    dis[s] = 0;
    pq.push(node(s, 0));
    while ((!pq.empty()))
    {
      var now = pq.top();
      pq.pop();
      var u = now.id;
      if ((dis[u] < now.val))
      {
        continue;
      }
      var len = cpp_cast(ed[u].size());
      {
        var i = 0;
        while ((i < len))
        {
          var v = ed[u][i].v;
          var f = ed[u][i].c;
          var w = ed[u][i].w;
          if ((ed[u][i].c && (dis[v] > (((dis[u] + w) + h[u]) - h[v]))))
          {
            dis[v] = (((dis[u] + w) + h[u]) - h[v]);
            fnasiofnoas[v] = u;
            pree[v] = i;
            pq.push(node(v, dis[v]));
          }
          i += 1;
        }
      }
    }
    if ((dis[t] == inf))
    {
      break;
    }
    {
      var i = 0;
      while ((i <= t))
      {
        h[i] += dis[i];
        i += 1;
      }
    }
    var flow = inf;
    {
      var i = t;
      while (i)
      {
        flow = min(flow, ed[fnasiofnoas[i]][pree[i]].c);
        i = fnasiofnoas[i];
      }
    }
    tot -= flow;
    res += (flow * h[t]);
    {
      var i = t;
      while (i)
      {
        var e = ed[fnasiofnoas[i]][pree[i]];
        e.c -= flow;
        ed[e.v][e.rev].c += flow;
        i = fnasiofnoas[i];
      }
    }
  }
  return res;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var c: dynamic;
  var d: dynamic;
  read(n, m, k, c, d);
  {
    var i = 1;
    while ((i <= k))
    {
      var x: dynamic;
      read(x);
      num[x] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      eee[u].push_back(v);
      eee[v].push_back(u);
      i += 1;
    }
  }
  s = 0;
  t = ((n * 100) + 1);
  {
    var i = 1;
    while ((i <= n))
    {
      addedge(s, i, num[i], 0);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= 99))
    {
      var N = (((i - 1)) * n);
      {
        var j = 1;
        while ((j <= n))
        {
          for (var v in eee[j])
          {
            {
              var z = 1;
              while ((z <= k))
              {
                addedge((j + N), ((v + N) + n), 1, (((((2 * z) - 1)) * d) + c));
                z += 1;
              }
            }
          }
          addedge((j + N), ((j + N) + n), inf, c);
          j += 1;
        }
      }
      addedge((N + 1), t, inf, 0);
      i += 1;
    }
  }
  addedge(((99 * n) + 1), t, inf, 0);
  printf("%d\n", costflow());
  return 0;
}
