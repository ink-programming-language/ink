// Translated from solution.cpp.

var N = (200 + 5);

var n: dynamic;

var k: dynamic;

var id = cpp_array(N);

var q: dynamic;

var map: dynamic;

func print()
{
  if (q.empty())
  {
    return;
  }
  var t = q.front();
  q.pop();
  print();
  printf("%d ", t);
}

func main()
{
  scanf("%d%d", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&id[i]));
      i += 1;
    }
  }
  {
    var i = 1;
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
