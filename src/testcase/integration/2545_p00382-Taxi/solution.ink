// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var i: dynamic;
  var j: dynamic;
  var m: dynamic;
  var c: dynamic;
  {
    i = 0;
    while ((i < N))
    {
      read(m);
      {
        j = 0;
        while ((j < m))
        {
          read(c);
          C[i].push_back(c);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var q: dynamic;
  var ans = 0;
  {
    i = 0;
    while ((i < N))
    {
      {
        j = 0;
        while ((j < C[((N - i) - 1)].size()))
        {
          q.push(C[((N - i) - 1)][j]);
          j += 1;
        }
      }
      ans += q.top();
      q.pop();
      i += 1;
    }
  }
  write(ans, "\n");
}
