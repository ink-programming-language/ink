// Translated from solution.cpp.

func debug(x: dynamic)
{
  cpp_macro(";");
}

func debug(x: dynamic)
{
  cpp_macro("cerr << __LINE__ << \" : \" << #x << \" = \" << (x) << endl;");
}

func operator_shift_left(out: dynamic, p: dynamic)
{
  (((((out << "{") << p.first) << ", ") << p.second) << "}");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  (out << cpp_char("{"));
  for (var item in v)
  {
    ((out << item) << ", ");
  }
  (out << "\u{8}\u{8}}");
  return out;
}

var mod = cpp_expression("#include <");

var INF = cpp_expression("#include <");

var LLINF = cpp_expression("#include <cstdio> #include <");

var SIZE = cpp_expression("#inclu");

class ACNode
{
  var val: dynamic;
  var next: dynamic;
  var failure: dynamic;
  var id: dynamic;
  func ACNode()
  {
      this->val = cpp_construct(0);
      memset(next, 0, cpp_sizeof((next)));
    }
  func insert(s: dynamic, id: dynamic)
  {
      id = id;
      if ((!(*s)))
      {
        val += 1;
        return;
      }
      var al = ((*s) - cpp_char("a"));
      if ((next[al] == null))
      {
        next[al] = cpp_new();
      }
      next[al]->insert((s + 1), (id + 1));
    }
  func nextNode(c: dynamic)
  {
      var al = (c - cpp_char("a"));
      if (next[al])
      {
        return next[al];
      }
      return if ((failure == this)) this else failure->nextNode(c);
    }
}

class AhoCorasick
{
  var node: dynamic;
  func AhoCorasick()
  {
      node = cpp_new();
    }
  func insert(s: dynamic)
  {
      node->insert(s, 0);
    }
  func build()
  {
      var que: dynamic;
      que.push(node);
      node->failure = node;
      while (que.size())
      {
        var p = que.front();
        que.pop();
        {
          var i = 0;
          while ((i < 26))
          {
            if (p->next[i])
            {
              var failure = p->failure;
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

func apply(a: dynamic, b: dynamic)
{
  var res: dynamic;
  assert((a.size() == b.size()));
  {
    var i = 0;
    while ((i < a.size()))
    {
      res.push_back(b[a[i]]);
      i += 1;
    }
  }
  return res;
}

func apply2(a: dynamic, b: dynamic)
{
  var res = cpp_construct(a.size(), 0);
  assert((a.size() == b.size()));
  {
    var i = 0;
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
  var segn2: dynamic;
  var data: dynamic;
  var rep: dynamic;
  var base: dynamic;
  func merge(a: dynamic, b: dynamic)
  {
      var res = a;
      {
        var i = 0;
        while ((i < a.size()))
        {
          res[i] += b[i];
          i += 1;
        }
      }
      return res;
    }
  func SegTree(n: dynamic, m: dynamic)
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
        var i = (segn2 - 1);
        while ((i < ((segn2 - 1) + n)))
        {
          data[i] = v;
          i += 1;
        }
      }
      {
        var i = (segn2 - 2);
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
  func query(a: dynamic, b: dynamic, l: dynamic = 0, r: dynamic = -1, k: dynamic = 0)
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
      var res1 = query(a, b, l, (((l + r)) / 2), ((k * 2) + 1));
      var res2 = query(a, b, (((l + r)) / 2), r, ((k * 2) + 2));
      var res12 = merge(res1, res2);
      return apply2(res12, rep[k]);
    }
  func add(a: dynamic, b: dynamic, x: dynamic, u: dynamic, l: dynamic = 0, r: dynamic = -1, k: dynamic = 0)
  {
      if ((r == -1))
      {
        r = segn2;
      }
      rep[k] = apply(rep[k], u);
      var res1: dynamic;
      var res2: dynamic;
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
        var v = merge(data[((k * 2) + 1)], data[((k * 2) + 2)]);
        data[k] = v;
      } else
      {
        data[k] = apply2(data[k], u);
      }
    }
}

func main()
{
  var S = cpp_array(21);
  var N: dynamic;
  var Q: dynamic;
  var M: dynamic;
  var start: dynamic;
  scanf("%s%d%d", S, (&N), (&Q));
  M = strlen(S);
  var aho: dynamic;
  aho.insert(S);
  aho.build();
  start[0] = aho.node;
  var seg = cpp_construct(N, (M + 1));
  {
    var i = 0;
    while ((i < M))
    {
      start[(i + 1)] = start[i]->nextNode(S[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Q))
    {
      var q: dynamic;
      var l: dynamic;
      var r: dynamic;
      var c = cpp_array(11);
      scanf("%d%d%d", (&q), (&l), (&r));
      l -= 1;
      if ((q == 1))
      {
        scanf("%s", c);
        var vec: dynamic;
        {
          var j = 0;
          while ((j <= M))
          {
            var p = start[j];
            {
              var k = 0;
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
        var res = seg.query(l, r);
        printf("%d\n", res[M]);
      }
      i += 1;
    }
  }
  return 0;
}
