// Translated from solution.cpp.

var eps = 1e-10;

var K: dynamic;

var R: dynamic;

var L: dynamic;

var P: dynamic;

var E: dynamic;

var T: dynamic;

func atime(l: dynamic, r: dynamic)
{
  if ((l > (LLONG_MAX / r)))
  {
    return LLONG_MAX;
  } else
  {
    return (l * r);
  }
}

func powint(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return 1;
  } else
  {
    return atime(a, powint(a, (b - 1)));
  }
}

func aplus(l: dynamic, r: dynamic)
{
  if (((l > (LLONG_MAX / 3)) || (R > LLONG_MAX)))
  {
    return LLONG_MAX;
  } else
  {
    return (l + r);
  }
}

func getans(sums: dynamic, tickets: dynamic, now: dynamic, ans: dynamic, rest: dynamic)
{
  if ((now == tickets.size()))
  {
    ans = min(ans, rest);
    return;
  }
  if ((ans < (rest - sums[now])))
  {
    return;
  }
  if ((rest >= tickets[now]))
  {
    getans(sums, tickets, (now + 1), ans, (rest - tickets[now]));
  } else
  {
    ans = min(ans, (tickets[now] - rest));
  }
  getans(sums, tickets, (now + 1), ans, rest);
}

func main()
{
  var N: dynamic;
  var M: dynamic;
  var A: dynamic;
  var B: dynamic;
  var P: dynamic;
  var Q: dynamic;
  read(N, M, A, B, P, Q);
  if ((A > B))
  {
    swap(A, B);
    swap(P, Q);
  }
  assert((A <= B));
  if (((A == 1) && (B == 1)))
  {
    var ticdis = (P + Q);
    if ((atime(ticdis, N) <= M))
    {
      write((M - atime(ticdis, N)), "\n");
    } else
    {
      write(min((ticdis - (M % ticdis)), (M % ticdis)), "\n");
    }
  } else
  {
    var tickets: dynamic;
    var ans = M;
    {
      var k = 0;
      while ((k < N))
      {
        var tic = aplus(atime(P, powint(A, k)), atime(Q, powint(B, k)));
        if ((tic > M))
        {
          ans = min(ans, (tic - M));
          break;
        } else
        {
          tickets.emplace_back(tic);
        }
        k += 1;
      }
    }
    reverse(tickets.begin(), tickets.end());
    {
      var i = (sums.size() - 1);
      while ((i > 0))
      {
        sums[(i - 1)] += sums[i];
        i -= 1;
      }
    }
    getans(sums, tickets, 0, ans, M);
    write(ans, "\n");
  }
  return 0;
}
