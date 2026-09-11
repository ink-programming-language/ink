// Translated from solution.cpp.

var MAXN: dynamic = 100001;

var arr: dynamic = cpp_array(MAXN);

var mp: dynamic = cpp_array(MAXN);

var loga: dynamic = cpp_array(MAXN);

func add(a: dynamic, b: dynamic, x: dynamic) -> dynamic
{
  {
    var i: dynamic = (a + 1);
    while ((i < MAXN))
    {
      {
        var j: dynamic = (b + 1);
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

func get(a: dynamic, b: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = (a + 1);
    while (i)
    {
      {
        var j: dynamic = (b + 1);
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

func remove(i: dynamic) -> dynamic
{
  var s: dynamic = mp[arr[i]];
  var it: dynamic = s.find(i);
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

func insert(i: dynamic) -> dynamic
{
  var s: dynamic = mp[arr[i]];
  var it: dynamic = s.insert(i).first;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
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
  for (var x: dynamic in mp)
  {
    if ((!x.empty()))
    {
      {
        var it: dynamic = next(x.begin());
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
    var q: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
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
