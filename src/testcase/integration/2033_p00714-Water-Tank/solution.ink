// Translated from solution.cpp.

var eps: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

var INF: dynamic = (INT_MAX / 2);

var LINF: dynamic = (LLONG_MAX / 2);

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  (((os << p.first) << ":") << p.second);
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((v.size())))))
    {
      if (i)
      {
        (os << " ");
      }
      (os << v[i]);
      i += 1;
    }
  }
  return os;
}

class Node
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var rem: dynamic = cpp_uninitialized();
  var lch: dynamic = cpp_uninitialized();
  var rch: dynamic = cpp_uninitialized();
  var par: dynamic = cpp_uninitialized();
  func Node(l: dynamic, r: dynamic, w: dynamic) -> dynamic
  {
      self->l = cpp_construct(l);
      self->r = cpp_construct(r);
      self->w = cpp_construct(w);
    }
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      read(h1[i].first, h1[i].second);
      h2[i].first = h1[i].second;
      h2[i].second = h1[i].first;
      i += 1;
    }
  }
  sort((h1).begin(), (h1).end());
  sort((h2).begin(), (h2).end());
  var m: dynamic = cpp_uninitialized();
  read(m);
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      read(f[i], a[i]);
      a[i] /= 30;
      i += 1;
    }
  }
  var l: dynamic = cpp_uninitialized();
  read(l);
  {
    var i: dynamic = 0;
    while ((i < (l)))
    {
      read(p[i].first.second, p[i].first.first);
      p[i].second = i;
      i += 1;
    }
  }
  sort((p).begin(), (p).end());
  var nodes: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      if ((i == 0))
      {
        nodes[i] = make_shared(Node(i, (i + 1), h1[i].first));
        nodes[i]->rem = (nodes[i]->h * nodes[i]->w);
      } else
      {
        nodes[i] = make_shared(Node(i, (i + 1), (h1[i].first - h1[(i - 1)].first)));
        nodes[i]->rem = (nodes[i]->h * nodes[i]->w);
      }
      i += 1;
    }
  }
  nodes[n] = make_shared(Node(n, (n + 1), (100 - h1[(n - 1)].first)));
  nodes[n]->rem = (nodes[n]->h * nodes[n]->w);
  var leaves: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 0;
    while ((i < ((n + 1))))
    {
      leaves[i] = nodes[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      var nowh: dynamic = h2[i].first;
      var nowb: dynamic = h2[i].second;
      var it: dynamic = lower_bound((h1).begin(), (h1).end(), make_pair(nowb, 0));
      var pos: dynamic = distance(h1.begin(), it);
      var node1: dynamic = nodes[pos];
      var node2: dynamic = nodes[(pos + 1)];
      var newnode: dynamic = make_shared(Node(node1->l, node2->r, (node1->w + node2->w)));
      node1->h += nowh;
      node2->h += nowh;
      node1->rem = (node1->h * node1->w);
      node2->rem = (node2->h * node2->w);
      newnode->h = (-nowh);
      newnode->lch = node1;
      newnode->rch = node2;
      node1->par = newnode;
      node2->par = newnode;
      {
        var j: dynamic = (node1->l);
        while ((j < (node2->r)))
        {
          nodes[j] = newnode;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var head: dynamic = nodes[0];
  head->h += 50;
  head->rem = (head->h * head->w);
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      var idx: dynamic = 0;
      if ((f[i] < h1[0].first))
      {
        idx = 0;
      } else
      {
        var it: dynamic = upper_bound((h1).begin(), (h1).end(), make_pair(f[i], 0));
        var pos: dynamic = distance(h1.begin(), it);
        idx = pos;
      }
      mp[i][leaves[idx]] = true;
      nodepos[i] = leaves[idx];
      i += 1;
    }
  }
  var nowt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (l)))
    {
      {
        var j: dynamic = 0;
        while ((j < (m)))
        {
          var diff: dynamic = (p[i].first.first - nowt);
          diff *= a[j];
          var nownode: dynamic = nodepos[j];
          while ((diff > eps))
          {
            if ((diff < (nownode->rem - eps)))
            {
              nownode->rem -= diff;
              nodepos[j] = nownode;
              break;
            }
            diff -= nownode->rem;
            nownode->rem = 0;
            if ((nownode->par == null))
            {
              break;
            }
            if ((nownode == nownode->par->lch))
            {
              nownode = nownode->par;
              if ((!mp[j][nownode->rch]))
              {
                nownode = nownode->rch;
                while ((nownode->lch != null))
                {
                  nownode = nownode->lch;
                }
              }
            } else if ((nownode == nownode->par->rch))
            {
              nownode = nownode->par;
              if ((!mp[j][nownode->lch]))
              {
                nownode = nownode->lch;
                while ((nownode->rch != null))
                {
                  nownode = nownode->rch;
                }
              }
            } else
            {
              nownode = nownode->par;
            }
            mp[j][nownode] = true;
            nodepos[j] = nownode;
          }
          j += 1;
        }
      }
      nowt = p[i].first.first;
      var it: dynamic = upper_bound((h1).begin(), (h1).end(), make_pair(p[i].first.second, 0));
      var pos: dynamic = distance(h1.begin(), it);
      var nownode: dynamic = leaves[pos];
      var ans: dynamic = 50;
      while (1)
      {
        ans -= (nownode->rem / nownode->w);
        if ((nownode->par == null))
        {
          break;
        } else
        {
          nownode = nownode->par;
        }
      }
      out[p[i].second] = ans;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (l)))
    {
      write(out[i], "\n");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  write(fixed, setprecision(10));
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
