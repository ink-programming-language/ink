// Translated from solution.cpp.

var MAXN: dynamic = cpp_expression("// GRL");

class cpp_class_1
{
  var to: dynamic = cpp_uninitialized();
  var wt: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var par: dynamic = cpp_array(MAXN);

var E: dynamic = cpp_array(MAXN);

var LE: dynamic = cpp_array(MAXN);

class Seg
{
  var v: dynamic = cpp_uninitialized();
  func Seg(in_cpp: dynamic) -> dynamic
  {
      self->v = cpp_construct(pow(2, (1 + ceil(log2(in_cpp.size())))));
      {
        var i: dynamic = 0;
        while ((i < in_cpp.size()))
        {
          v[((v.size() / 2) + i)] = par[in_cpp[i]].wt;
          i += 1;
        }
      }
      {
        var k: dynamic = ((v.size() / 2) - 1);
        while ((k > 0))
        {
          v[k] = (v[(2 * k)] + v[((2 * k) + 1)]);
          k -= 1;
        }
      }
    }
  func Add(i: dynamic, x: dynamic) -> dynamic
  {
      i += (v.size() / 2);
      v[i] += x;
      while (cpp_assign(i, "=", (i / 2)))
      {
        v[i] = (v[(2 * i)] + v[((2 * i) + 1)]);
      }
    }
  func Get(i: dynamic) -> dynamic
  {
      i += (v.size() / 2);
      var rs: dynamic = 0;
      while (i)
      {
        if ((i == 1))
        {
          return v[1];
        }
        if ((i % 2))
        {
          i /= 2;
        } else
        {
          rs += v[i];
          if ((!((((i / 2)) & (((i / 2) - 1))))))
          {
            return rs;
          }
          i = (((i / 2)) - 1);
        }
      }
      return rs;
    }
}

var aux: dynamic = cpp_uninitialized();

var tidx: dynamic = cpp_array(MAXN);

var jump: dynamic = cpp_array(MAXN);

var size: dynamic = cpp_array(MAXN);

func Build(i: dynamic, a: dynamic) -> dynamic
{
  a.push_back(i);
  {
    var j: dynamic = LE[i];
    while ((j != -1))
    {
      if ((size[E[j].to] > (size[i] / 2)))
      {
        Build(E[j].to, a);
        aux[i] = aux[E[j].to];
        jump[i] = jump[E[j].to];
        tidx[i] = (tidx[E[j].to] - 1);
      } else
      {
        var newa: dynamic = cpp_uninitialized();
        Build(E[j].to, newa);
      }
      j = E[j].next;
    }
  }
  if (((a.size() > 1) && (!aux[i])))
  {
    aux[i] = cpp_new(a);
    jump[i] = par[a.front()].to;
    tidx[i] = (a.size() - 1);
  }
}

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  fill(LE, (LE + n), -1);
  fill(size, (size + n), 1);
  fill(jump, (jump + n), -1);
  fill(aux, (aux + n), null);
  par[0] = [-1, 0, -1];
  {
    var s: dynamic = 0;
    var i: dynamic = 0;
    while ((s < n))
    {
      var k: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      read(k);
      while ((cpp_update(k, "--") > 0))
      {
        read(t);
        par[t] = [s, 0, -1];
        E[i] = [t, 0, LE[s]];
        LE[s] = i;
        i += 1;
      }
      s += 1;
    }
  }
  var dfs: dynamic = cpp_uninitialized();
  dfs.push(0);
  while ((!dfs.empty()))
  {
    var u: dynamic = dfs.top();
    dfs.pop();
    if ((visited[u] && (par[u].to >= 0)))
    {
      size[par[u].to] += size[u];
    }
    if ((!visited[u]))
    {
      visited[u] = true;
      dfs.push(u);
      {
        var j: dynamic = LE[u];
        while ((j != -1))
        {
          dfs.push(E[j].to);
          j = E[j].next;
        }
      }
    }
  }
  var a: dynamic = cpp_uninitialized();
  Build(0, a);
  var q: dynamic = cpp_uninitialized();
  read(q);
  {
    var kind: dynamic = cpp_uninitialized();
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    while (((cin >> kind) >> u))
    {
      if ((kind == 0))
      {
        read(v);
        if (aux[u])
        {
          aux[u]->Add(tidx[u], v);
        } else
        {
          par[u].wt += v;
        }
      } else
      {
        var rs: dynamic = 0;
        while ((u > 0))
        {
          if (aux[u])
          {
            rs += aux[u]->Get(tidx[u]);
            u = jump[u];
          } else
          {
            rs += par[u].wt;
            u = par[u].to;
          }
        }
        write(rs, "\n");
      }
    }
  }
}
