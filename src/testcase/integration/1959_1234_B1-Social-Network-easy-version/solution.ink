// Translated from solution.cpp.

var N: dynamic = (200 + 5);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var id: dynamic = cpp_array(N);

var q: dynamic = cpp_uninitialized();

var map: dynamic = cpp_uninitialized();

func print() -> dynamic
{
  if (q.empty())
  {
    return;
  }
  var t: dynamic = q.front();
  q.pop();
  print();
  printf("%d ", t);
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&id[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (map[id[i]])
      {
        i += 1;
        continue;
      }
      if ((q.size() == k))
      {
        map[q.front()] = 0;
        q.pop();
      }
      q.push(id[i]);
      map[id[i]] = 1;
      i += 1;
    }
  }
  printf("%d\n", q.size());
  print();
  return 0;
}
