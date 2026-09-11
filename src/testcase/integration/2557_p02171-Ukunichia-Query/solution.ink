// Translated from solution.cpp.

func debug(x: dynamic) -> dynamic
{
  cpp_macro(";");
}

func debug(x: dynamic) -> dynamic
{
  cpp_macro("cerr << __LINE__ << \" : \" << #x << \" = \" << (x) << endl;");
}

func operator_shift_left(out: dynamic, p: dynamic) -> dynamic
{
  (((((out << "{") << p.first) << ", ") << p.second) << "}");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  (out << cpp_char("{"));
  for (var item: dynamic in v)
  {
    ((out << item) << ", ");
  }
  (out << "\u{8}\u{8}}");
  return out;
}

var mod: dynamic = cpp_expression("#include <");

var INF: dynamic = cpp_expression("#include <");

var LLINF: dynamic = cpp_expression("#include <cstdio> #include <");

var SIZE: dynamic = cpp_expression("#inclu");

class ACNode
{
  var val: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  var failure: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func ACNode() -> dynamic
  {
      self->val = cpp_construct(0);
      memset(next, 0, cpp_sizeof((next)));
    }
  func insert(s: dynamic, id: dynamic) -> dynamic
  {
      id = id;
      if ((!(*s)))
      {
        val += 1;
        return;
      }
      var al: dynamic = ((*s) - cpp_char("a"));
      if ((next[al] == null))
      {
        next[al] = cpp_new();
      }
      next[al]->insert((s + 1), (id + 1));
    }
  func nextNode(c: dynamic) -> dynamic
  {
      var al: dynamic = (c - cpp_char("a"));
      if (next[al])
      {
        return next[al];
      }
      return  ((failure == self)) ? self : failure->nextNode(c);
    }
}

class AhoCorasick
{
  var node: dynamic = cpp_uninitialized();
  func AhoCorasick() -> dynamic
  {
      node = cpp_new();
    }
  func insert(s: dynamic) -> dynamic
  {
      node->insert(s, 0);
    }
  func build() -> dynamic
  {
      var que: dynamic = cpp_uninitialized();
      que.push(node);
      node->failure = node;
      while (que.size())
      {
        var p: dynamic = que.front();
        que.pop();
        {
          var i: dynamic = 0;
          while ((i < 26))
          {
            if (p->next[i])
            {
              var failure: dynamic = p->failure;
              while (((!failure->next[i]) && (failure != node)))
              {
                failure = failure->failure;
              }
              if ((failure->next[i] && (failure != p)))
              {
                p->next[i]->failure = failure->next[i];
                p->next[i]->val += failure->next[i]->val;
              } else
              {
                p->next[i]->failure = node;
              }
              que.push(p->next[i]);
            }
            i += 1;
          }
        }
      }
    }
}

func apply(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = cpp_uninitialized();
  assert((a.size() == b.size()));
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      res.push_back(b[a[i]]);
      i += 1;
    }
  }
  return res;
}

func apply2(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = cpp_construct(a.size(), 0);
  assert((a.size() == b.size()));
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      res[b[i]] += a[i];
      i += 1;
    }
  }
  return res;
}

class SegTree
{
  var segn2: dynamic = cpp_uninitialized();
  var data: dynamic = cpp_uninitialized();
  var rep: dynamic = cpp_uninitialized();
  var base: dynamic = cpp_uninitialized();
  func merge(a: dynamic, b: dynamic) -> dynamic
  {
      var res: dynamic = a;
      {
        var i: dynamic = 0;
        while ((i < a.size()))
        {
          res[i] += b[i];
          i += 1;
        }
      }
      return res;
    }
  func SegTree(n: dynamic, m: dynamic) -> dynamic
  {
      {
        segn2 = 1;
        while ((segn2 < n))
        {
          segn2 *= 2;
        }
      }
      data.assign((segn2 * 2), v);
      v[0] = 1;
      {
        var i: dynamic = (segn2 - 1);
        while ((i < ((segn2 - 1) + n)))
        {
          data[i] = v;
          i += 1;
        }
      }
      {
        var i: dynamic = (segn2 - 2);
        while ((i >= 0))
        {
          data[i] = merge(data[((i * 2) + 1)], data[((i * 2) + 2)]);
          i -= 1;
        }
      }
      base.assign(m, 0);
      iota(base.begin(), base.end(), 0);
      rep.assign((segn2 * 2), base);
    }
  func query(a: dynamic, b: dynamic, l: dynamic = 0, r: dynamic = -1, k: dynamic = 0) -> dynamic
  {
      if ((r == -1))
      {
        r = segn2;
      }
      if (((r <= a) || (b <= l)))
      {
        return vector(data[k].size(), 0);
      }
      if (((a <= l) && (r <= b)))
      {
        return data[k];
      }
      var res1: dynamic = query(a, b, l, (((l + r)) / 2), ((k * 2) + 1));
      var res2: dynamic = query(a, b, (((l + r)) / 2), r, ((k * 2) + 2));
      var res12: dynamic = merge(res1, res2);
      return apply2(res12, rep[k]);
    }
  func add(a: dynamic, b: dynamic, x: dynamic, u: dynamic, l: dynamic = 0, r: dynamic = -1, k: dynamic = 0) -> dynamic
  {
      if ((r == -1))
      {
        r = segn2;
      }
      rep[k] = apply(rep[k], u);
      var res1: dynamic = cpp_uninitialized();
      var res2: dynamic = cpp_uninitialized();
      if (((a <= l) && (r <= b)))
      {
        rep[k] = apply(rep[k], x);
        data[k] = apply2(data[k], u);
        data[k] = apply2(data[k], x);
      } else if (((a < r) && (l < b)))
      {
        add(a, b, x, rep[k], l, (((l + r)) / 2), ((k * 2) + 1));
        add(a, b, x, rep[k], (((l + r)) / 2), r, ((k * 2) + 2));
        rep[k] = base;
        var v: dynamic = merge(data[((k * 2) + 1)], data[((k * 2) + 2)]);
        data[k] = v;
      } else
      {
        data[k] = apply2(data[k], u);
      }
    }
}

func main() -> dynamic
{
  var S: dynamic = cpp_array(21);
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var start: dynamic = cpp_uninitialized();
  scanf("%s%d%d", S, (&N), (&Q));
  M = strlen(S);
  var aho: dynamic = cpp_uninitialized();
  aho.insert(S);
  aho.build();
  start[0] = aho.node;
  var seg: dynamic = cpp_construct(N, (M + 1));
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      start[(i + 1)] = start[i]->nextNode(S[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < Q))
    {
      var q: dynamic = cpp_uninitialized();
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_array(11);
      scanf("%d%d%d", (&q), (&l), (&r));
      l -= 1;
      if ((q == 1))
      {
        scanf("%s", c);
        var vec: dynamic = cpp_uninitialized();
        {
          var j: dynamic = 0;
          while ((j <= M))
          {
            var p: dynamic = start[j];
            {
              var k: dynamic = 0;
              while (c[k])
              {
                p = p->nextNode(c[k]);
                k += 1;
              }
            }
            vec.push_back(p->id);
            j += 1;
          }
        }
        seg.add(l, r, vec, seg.base);
      } else
      {
        var res: dynamic = seg.query(l, r);
        printf("%d\n", res[M]);
      }
      i += 1;
    }
  }
  return 0;
}
