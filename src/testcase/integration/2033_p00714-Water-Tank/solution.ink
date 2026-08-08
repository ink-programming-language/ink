// Translated from solution.cpp.

var eps = 1e-8;

var MOD = 1000000007;

var INF = (INT_MAX / 2);

var LINF = (LLONG_MAX / 2);

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  (((os << p.first) << ":") << p.second);
  return os;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  {
    var i = 0;
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
  var h: dynamic;
  var w: dynamic;
  var l: dynamic;
  var r: dynamic;
  var rem: dynamic;
  var lch: dynamic;
  var rch: dynamic;
  var par: dynamic;
  func Node(l: dynamic, r: dynamic, w: dynamic)
  {
      this->l = cpp_construct(l);
      this->r = cpp_construct(r);
      this->w = cpp_construct(w);
    }
}

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
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
  var m: dynamic;
  read(m);
  {
    var i = 0;
    while ((i < (m)))
    {
      read(f[i], a[i]);
      a[i] /= 30;
      i += 1;
    }
  }
  var l: dynamic;
  read(l);
  {
    var i = 0;
    while ((i < (l)))
    {
      read(p[i].first.second, p[i].first.first);
      p[i].second = i;
      i += 1;
    }
  }
  sort((p).begin(), (p).end());
  var nodes = cpp_construct((n + 1));
  {
    var i = 0;
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
  var leaves = cpp_construct((n + 1));
  {
    var i = 0;
    while ((i < ((n + 1))))
    {
      leaves[i] = nodes[i];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (n)))
    {
      var nowh = h2[i].first;
      var nowb = h2[i].second;
      var it = lower_bound((h1).begin(), (h1).end(), make_pair(nowb, 0));
      var pos = distance(h1.begin(), it);
      var node1 = nodes[pos];
      var node2 = nodes[(pos + 1)];
      var newnode = make_shared(Node(node1->l, node2->r, (node1->w + node2->w)));
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
        var j = (node1->l);
        while ((j < (node2->r)))
        {
          nodes[j] = newnode;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var head = nodes[0];
  head->h += 50;
  head->rem = (head->h * head->w);
  {
    var i = 0;
    while ((i < (m)))
    {
      var idx = 0;
      if ((f[i] < h1[0].first))
      {
        idx = 0;
      } else
      {
        var it = upper_bound((h1).begin(), (h1).end(), make_pair(f[i], 0));
        var pos = distance(h1.begin(), it);
        idx = pos;
      }
      mp[i][leaves[idx]] = true;
      nodepos[i] = leaves[idx];
      i += 1;
    }
  }
  var nowt = 0;
  {
    var i = 0;
    while ((i < (l)))
    {
      {
        var j = 0;
        while ((j < (m)))
        {
          var diff = (p[i].first.first - nowt);
          diff *= a[j];
          var nownode = nodepos[j];
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
      var it = upper_bound((h1).begin(), (h1).end(), make_pair(p[i].first.second, 0));
      var pos = distance(h1.begin(), it);
      var nownode = leaves[pos];
      var ans = 50;
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
    var i = 0;
    while ((i < (l)))
    {
      write(out[i], "\n");
      i += 1;
    }
  }
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  write(fixed, setprecision(10));
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
