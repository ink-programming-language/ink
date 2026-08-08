// Translated from solution.cpp.

var MAXN = 100001;

var arr = cpp_array(MAXN);

var mp = cpp_array(MAXN);

var loga = cpp_array(MAXN);

func add(a: dynamic, b: dynamic, x: dynamic)
{
  {
    var i = (a + 1);
    while ((i < MAXN))
    {
      {
        var j = (b + 1);
        while ((j < MAXN))
        {
          loga[i][j] += x;
          j += (j & (-j));
        }
      }
      i += (i & (-i));
    }
  }
}

func get(a: dynamic, b: dynamic)
{
  var ret = 0;
  {
    var i = (a + 1);
    while (i)
    {
      {
        var j = (b + 1);
        while (j)
        {
          if (loga[i].count(j))
          {
            ret += loga[i][j];
          }
          j -= (j & (-j));
        }
      }
      i -= (i & (-i));
    }
  }
  return ret;
}

func remove(i: dynamic)
{
  var s = mp[arr[i]];
  var it = s.find(i);
  if ((it != s.begin()))
  {
    add((*prev(it)), (*it), ((*prev(it)) - (*it)));
  }
  if ((next(it) != s.end()))
  {
    add((*it), (*next(it)), ((*it) - (*next(it))));
  }
  if (((it != s.begin()) && (next(it) != s.end())))
  {
    add((*prev(it)), (*next(it)), ((*next(it)) - (*prev(it))));
  }
  s.erase(it);
}

func insert(i: dynamic)
{
  var s = mp[arr[i]];
  var it = s.insert(i).first;
  if ((it != s.begin()))
  {
    add((*prev(it)), (*it), ((*it) - (*prev(it))));
  }
  if ((next(it) != s.end()))
  {
    add((*it), (*next(it)), ((*next(it)) - (*it)));
  }
  if (((it != s.begin()) && (next(it) != s.end())))
  {
    add((*prev(it)), (*next(it)), ((*prev(it)) - (*next(it))));
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  read(n, m);
  {
    i = 0;
    while ((i < n))
    {
      read(arr[i]);
      mp[arr[i]].insert(i);
      i += 1;
    }
  }
  for (var x in mp)
  {
    if ((!x.empty()))
    {
      {
        var it = next(x.begin());
        while ((it != x.end()))
        {
          add((*prev(it)), (*it), ((*it) - (*prev(it))));
          it += 1;
        }
      }
    }
  }
  while (cpp_update(m, "--"))
  {
    var q: dynamic;
    var a: dynamic;
    var b: dynamic;
    read(q, a, b);
    a -= 1;
    if ((q == 1))
    {
      remove(a);
      arr[a] = b;
      insert(a);
    } else
    {
      a -= 1;
      b -= 1;
      write((((get(b, b) - get(a, b)) - get(b, a)) + get(a, a)), cpp_char("\n"));
    }
  }
}
