// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

class Pair
{
  var id: dynamic;
  var v: dynamic;
  func Pair()
  {
      id = cpp_assign(v, "=", 0);
    }
  func Pair(a: dynamic, b: dynamic)
  {
      id = a;
      v = b;
    }
}

var q: dynamic;

var p: dynamic;

class node
{
  var x: dynamic;
  var y: dynamic;
  var v: dynamic;
  var id: dynamic;
}

var a = cpp_array(200010);

var b = cpp_array(200010);

func cmp1(a: dynamic, b: dynamic)
{
  return if ((a.x == b.x)) (a.y < b.y) else (a.x < b.x);
}

func cmp2(a: dynamic, b: dynamic)
{
  return if ((a.y == b.y)) (a.x > b.x) else (a.y > b.y);
}

class Splay
{
  var tr: dynamic = cpp_array(200010);
  var tot: dynamic;
  var root: dynamic;
  func Splay()
  {
      tot = 0;
      root = 0;
      tr[0].v.v = cpp_assign(tr[0].mx.v, "=", 0);
    }
  func update(x: dynamic)
  {
      tr[x].mx = tr[x].v;
      if ((tr[x].ch[0] && (tr[x].mx < tr[tr[x].ch[0]].mx)))
      {
        tr[x].mx = tr[tr[x].ch[0]].mx;
      }
      if ((tr[x].ch[1] && (tr[x].mx < tr[tr[x].ch[1]].mx)))
      {
        tr[x].mx = tr[tr[x].ch[0]].mx;
      }
      tr[x].size = 1;
      if (tr[x].ch[0])
      {
        tr[x].size += tr[tr[x].ch[0]].size;
      }
      if (tr[x].ch[1])
      {
        tr[x].size += tr[tr[x].ch[1]].size;
      }
    }
  func rotate(x: dynamic, b: dynamic)
  {
      var y = tr[x].pnt;
      var z = tr[y].pnt;
      var son = tr[x].ch[b];
      if (son)
      {
        tr[son].pnt = y;
      }
      tr[y].pnt = x;
      tr[x].pnt = z;
      if (z)
      {
        tr[z].ch[(tr[z].ch[1] == y)] = x;
      }
      tr[x].ch[b] = y;
      tr[y].ch[(!b)] = son;
      update(y);
    }
  func splay(x: dynamic, target: dynamic)
  {
      while ((tr[x].pnt != target))
      {
        var y = tr[x].pnt;
        if ((tr[y].pnt == target))
        {
          rotate(x, (tr[y].ch[0] == x));
        } else
        {
          var z = tr[y].pnt;
          var c = (tr[y].ch[0] == x);
          var d = (tr[z].ch[0] == y);
          if ((c == d))
          {
            rotate(y, c);
            rotate(x, c);
          } else
          {
            rotate(x, c);
            rotate(x, d);
          }
        }
      }
      update(x);
      if ((target == 0))
      {
        root = x;
      }
    }
  func insert(id: dynamic, key: dynamic, val: dynamic)
  {
      var x = root;
      if ((!root))
      {
        root = cpp_update(tot, "++");
        tr[tot].pnt = 0;
        tr[tot].key = key;
        tr[tot].v = Pair(id, val);
        update(tot);
        return;
      }
      while (1)
      {
        if ((tr[x].key == key))
        {
          tr[x].v = max(tr[x].v, Pair(id, val));
          update(x);
          return;
        }
        if ((tr[x].key < key))
        {
          if ((!tr[x].ch[1]))
          {
            tot += 1;
            tr[x].ch[1] = tot;
            tr[tot].pnt = x;
            tr[tot].key = key;
            tr[tot].v = Pair(id, val);
            update(tot);
            splay(tot, 0);
            return;
          } else
          {
            x = tr[x].ch[1];
          }
        }
        if ((tr[x].key > key))
        {
          if ((!tr[x].ch[0]))
          {
            tot += 1;
            tr[x].ch[0] = tot;
            tr[tot].key = key;
            tr[tot].pnt = x;
            tr[tot].v = Pair(id, val);
            update(tot);
            splay(tot, 0);
            return;
          } else
          {
            x = tr[x].ch[0];
          }
        }
      }
    }
  func query(x: dynamic, key: dynamic)
  {
      if ((!x))
      {
        return Pair(0, 0);
      }
      var l = tr[x].ch[0];
      var r = tr[x].ch[1];
      var res: dynamic;
      if ((tr[x].key < key))
      {
        if (r)
        {
          return query(r, key);
        } else
        {
          return Pair(0, 0);
        }
      } else if ((tr[x].key > key))
      {
        if (l)
        {
          return max(query(l, key), max(tr[r].mx, tr[x].v));
        } else
        {
          return max(tr[r].mx, tr[x].v);
        }
      } else if ((tr[x].key == key))
      {
        return max(tr[r].mx, tr[x].v);
      }
    }
}

var sp: dynamic;

func main()
{
  var ansx: dynamic;
  var ansy: dynamic;
  var ans = 0;
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&a[i].x), (&a[i].y));
      a[i].id = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&b[i].x), (&b[i].y), (&b[i].v));
      b[i].id = i;
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n), cmp1);
  sort((b + 1), ((b + 1) + m), cmp1);
  {
    var i = 1;
    var j = 1;
    while ((i <= m))
    {
      {
        while (((j <= n) && (a[j].x <= b[i].x)))
        {
          q.push(Pair(a[j].id, a[j].y));
          j += 1;
        }
      }
      if ((!q.size()))
      {
        i += 1;
        continue;
      }
      var tmp = ((1 * ((min(q.top().v, b[i].y) - b[i].x))) * b[i].v);
      if ((tmp > ans))
      {
        ans = tmp;
        ansx = q.top().id;
        ansy = b[i].id;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    var j = 1;
    while ((i <= n))
    {
      {
        while (((j <= m) && (b[j].x <= a[i].x)))
        {
          sp.insert(b[j].id, b[j].y, b[j].v);
          j += 1;
        }
      }
      var re = sp.query(sp.root, a[i].y);
      var tmp = ((1 * ((a[i].y - a[i].x))) * re.v);
      if ((tmp > ans))
      {
        ans = tmp;
        ansx = a[i].id;
        ansy = re.id;
      }
      i += 1;
    }
  }
  sort((a + 1), ((a + 1) + n), cmp2);
  sort((b + 1), ((b + 1) + m), cmp2);
  {
    var i = 1;
    var j = 1;
    while ((i <= m))
    {
      {
        while (((j <= n) && (a[j].y >= b[i].y)))
        {
          p.push(Pair(a[j].id, a[j].x));
          j += 1;
        }
      }
      if ((!p.size()))
      {
        i += 1;
        continue;
      }
      var tmp = ((1 * ((b[i].y - max(p.top().v, b[i].x)))) * b[i].v);
      if ((tmp > ans))
      {
        ans = tmp;
        ansx = p.top().id;
        ansy = b[i].id;
      }
      i += 1;
    }
  }
  printf("%I64d\n", ans);
  if ((ans > 0))
  {
    printf("%d %d", ansx, ansy);
  }
  return 0;
}
