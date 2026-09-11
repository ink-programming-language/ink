// Translated from solution.cpp.

var X: dynamic = cpp_array(200005);

var Y: dynamic = cpp_array(200005);

var W: dynamic = cpp_array(200005);

var S: dynamic = cpp_array(200005);

var colormark: dynamic = cpp_array(200005);

var mark: dynamic = cpp_array(200005);

var V: dynamic = cpp_array(200005);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var pq: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(W[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(X[i], Y[i]);
      S[X[i]] += 1;
      S[Y[i]] += 1;
      V[X[i]].push_back(i);
      V[Y[i]].push_back(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (S[i])
      {
        pq.insert([(S[i] - W[i]), i]);
      }
      i += 1;
    }
  }
  while (pq.size())
  {
    var q: dynamic = ((*pq.begin()));
    pq.erase(pq.begin());
    if ((q.first > 0))
    {
      write("DEAD", cpp_char("\n"));
      exit(0);
    }
    var id: dynamic = q.second;
    var wt: dynamic = cpp_uninitialized();
    for (var j: dynamic in V[id])
    {
      if (mark[j])
      {
        continue;
      }
      ans.push_back(j);
      if ((X[j] == id))
      {
        swap(X[j], Y[j]);
      }
      wt.push_back(X[j]);
      mark[j] = 1;
    }
    for (var j: dynamic in wt)
    {
      pq.erase([(S[j] - W[j]), j]);
      S[j] -= 1;
      if ((S[j] > 0))
      {
        pq.insert([(S[j] - W[j]), j]);
      }
    }
  }
  reverse(ans.begin(), ans.end());
  write("ALIVE", cpp_char("\n"));
  for (var j: dynamic in ans)
  {
    write((j + 1), " ");
  }
}
