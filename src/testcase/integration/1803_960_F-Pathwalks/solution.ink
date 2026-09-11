// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var nod: dynamic = cpp_array(100005);

func query(w: dynamic, m: dynamic) -> dynamic
{
  var it: dynamic = m.lower_bound(w);
  if ((it == m.begin()))
  {
    return 0;
  }
  it = prev(it);
  return it->second;
}

func upd(val: dynamic, w: dynamic, m: dynamic) -> dynamic
{
  var it: dynamic = m.upper_bound(w);
  if ((it != m.begin()))
  {
    it = prev(it);
    if ((val <= it->second))
    {
      return;
    }
  }
  while (true)
  {
    var it: dynamic = m.upper_bound(w);
    if ((it == m.end()))
    {
      break;
    }
    if ((it->second > val))
    {
      break;
    }
    m.erase(it);
  }
  m[w] = val;
}

func main() -> dynamic
{
  read(n, m);
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y, w);
      var len: dynamic = query(w, nod[x]);
      upd((len + 1), w, nod[y]);
      ans = max(ans, (len + 1));
      i += 1;
    }
  }
  write(ans);
  return 0;
}
