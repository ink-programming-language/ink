// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var maxm: dynamic = (5e3 + 10);

var inf: dynamic = 0x3f3f3f3f;

var SIZE: dynamic = (((1 << 21)) + 1);

var ibuf: dynamic = cpp_array(SIZE);

var iS: dynamic = cpp_uninitialized();

var iT: dynamic = cpp_uninitialized();

var obuf: dynamic = cpp_array(SIZE);

var oS: dynamic = obuf;

var oT: dynamic = ((oS + SIZE) - 1);

var c: dynamic = cpp_uninitialized();

var qu: dynamic = cpp_array(55);

var f: dynamic = cpp_uninitialized();

var qr: dynamic = cpp_uninitialized();

func flush() -> dynamic
{
  fwrite(obuf, 1, (oS - obuf), stdout);
  oS = obuf;
}

func putc(x: dynamic) -> dynamic
{
  (*cpp_update(oS, "++")) = x;
  if ((oS == oT))
  {
    flush();
  }
}

func read(x: dynamic) -> dynamic
{
  {
    f = 1;
    c = ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++")));
    while (((c < cpp_char("0")) || (c > cpp_char("9"))))
    {
      if ((c == cpp_char("-")))
      {
        f = -1;
      } else if ((c == EOF))
      {
        return 0;
      }
      c = ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++")));
    }
  }
  {
    x = 0;
    while (((c <= cpp_char("9")) && (c >= cpp_char("0"))))
    {
      x = ((x * 10) + ((c & 15)));
      c = ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++")));
    }
  }
  x *= f;
  return 1;
}

func read(x: dynamic) -> dynamic
{
  while (((((cpp_assign(x, "=", ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++"))))) == cpp_char(" ")) || (x == cpp_char("\n"))) || (x == cpp_char("\r"))))
  {
  }
  return (x != EOF);
}

func read(x: dynamic) -> dynamic
{
  while (((((cpp_assign((*x), "=", ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++"))))) == cpp_char("\n")) || ((*x) == cpp_char(" "))) || ((*x) == cpp_char("\r"))))
  {
  }
  if (((*x) == EOF))
  {
    return 0;
  }
  while ((!((((((*x) == cpp_char("\n")) || ((*x) == cpp_char(" "))) || ((*x) == cpp_char("\r"))) || ((*x) == EOF)))))
  {
    (*(cpp_update(x, "++"))) = ( ((iS == iT)) ? (cpp_assign(iT, "=", ((cpp_assign(iS, "=", ibuf)) + fread(ibuf, 1, SIZE, stdin))( ((iS == iT)) ? EOF : (*cpp_update(iS, "++"))))) : (*cpp_update(iS, "++")));
  }
  (*x) = 0;
  return 1;
}

func read(x: dynamic, y: dynamic...) -> dynamic
{
  return (read(x) && read(cpp_expand(y)));
}

func write(x: dynamic) -> dynamic
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

func write(x: dynamic) -> dynamic
{
  putc(x);
  return 0;
}

func write(x: dynamic) -> dynamic
{
  while ((*x))
  {
    putc((*x));
    x += 1;
  }
  return 0;
}

func write(x: dynamic) -> dynamic
{
  while ((*x))
  {
    putc((*x));
    x += 1;
  }
  return 0;
}

func write(x: dynamic, y: dynamic...) -> dynamic
{
  return (write(x) || write(cpp_expand(y)));
}

class Flusher
{
  func cpp_destruct_Flusher() -> dynamic
  {
      flush();
    }
}

var io_flusher: dynamic = cpp_uninitialized();

var eee: dynamic = cpp_array(maxm);

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var dis: dynamic = cpp_array(maxm);

var h: dynamic = cpp_array(maxm);

var fnasiofnoas: dynamic = cpp_array(maxm);

var pree: dynamic = cpp_array(maxm);

var num: dynamic = cpp_array(maxm);

class edge
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  func edge(a: dynamic = -1, b: dynamic = 0, cc: dynamic = 0, d: dynamic = 0, f: dynamic = 0) -> dynamic
  {
      self->rev = cpp_construct(a);
      self->u = cpp_construct(b);
      self->v = cpp_construct(cc);
      self->c = cpp_construct(d);
      self->w = cpp_construct(f);
    }
}

var ed: dynamic = cpp_array(maxm);

func addedge(u: dynamic, v: dynamic, c: dynamic, w: dynamic) -> dynamic
{
  ed[u].push_back(edge(cpp_cast(ed[v].size()), u, v, c, w));
  ed[v].push_back(edge((cpp_cast(ed[u].size()) - 1), v, u, 0, (-w)));
}

class node
{
  var id: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
  func node(a: dynamic = 0, b: dynamic = 0) -> dynamic
  {
      self->id = cpp_construct(a);
      self->val = cpp_construct(b);
    }
}

var pq: dynamic = cpp_uninitialized();

func costflow() -> dynamic
{
  var res: dynamic = 0;
  memset(h, 0, cpp_sizeof((h)));
  var tot: dynamic = inf;
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
      var now: dynamic = pq.top();
      pq.pop();
      var u: dynamic = now.id;
      if ((dis[u] < now.val))
      {
        continue;
      }
      var len: dynamic = cpp_cast(ed[u].size());
      {
        var i: dynamic = 0;
        while ((i < len))
        {
          var v: dynamic = ed[u][i].v;
          var f: dynamic = ed[u][i].c;
          var w: dynamic = ed[u][i].w;
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
      var i: dynamic = 0;
      while ((i <= t))
      {
        h[i] += dis[i];
        i += 1;
      }
    }
    var flow: dynamic = inf;
    {
      var i: dynamic = t;
      while (i)
      {
        flow = min(flow, ed[fnasiofnoas[i]][pree[i]].c);
        i = fnasiofnoas[i];
      }
    }
    tot -= flow;
    res += (flow * h[t]);
    {
      var i: dynamic = t;
      while (i)
      {
        var e: dynamic = ed[fnasiofnoas[i]][pree[i]];
        e.c -= flow;
        ed[e.v][e.rev].c += flow;
        i = fnasiofnoas[i];
      }
    }
  }
  return res;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, m, k, c, d);
  {
    var i: dynamic = 1;
    while ((i <= k))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      num[x] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      eee[u].push_back(v);
      eee[v].push_back(u);
      i += 1;
    }
  }
  s = 0;
  t = ((n * 100) + 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      addedge(s, i, num[i], 0);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= 99))
    {
      var N: dynamic = (((i - 1)) * n);
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          for (var v: dynamic in eee[j])
          {
            {
              var z: dynamic = 1;
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
