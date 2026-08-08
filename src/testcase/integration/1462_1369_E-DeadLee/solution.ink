// Translated from solution.cpp.

var X = cpp_array(200005);

var Y = cpp_array(200005);

var W = cpp_array(200005);

var S = cpp_array(200005);

var colormark = cpp_array(200005);

var mark = cpp_array(200005);

var V = cpp_array(200005);

var ans: dynamic;

func main()
{
  var pq: dynamic;
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(W[i]);
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 1;
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
    var q = ((*pq.begin()));
    pq.erase(pq.begin());
    if ((q.first > 0))
    {
      write("DEAD", cpp_char("\n"));
      exit(0);
    }
    var id = q.second;
    var wt: dynamic;
    for (var j in V[id])
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
    for (var j in wt)
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
  for (var j in ans)
  {
    write((j + 1), " ");
  }
}
