// Translated from solution.cpp.

var inf: dynamic;

var inf = 1e9;

var inf = 1e18;

var M = (1e9 + 7);

class aho_corasick
{
  func aho_corasick(ts: dynamic)
  {
      this->K = cpp_construct(ts.size());
      this->root = cpp_construct(make_shared());
      root->fail = root;
      {
        var i = 0;
        while ((i < K))
        {
          var t = root.get();
          for (var cc in ts[i])
          {
            var c = (cc - alphabet_base);
            if ((!t->next[c]))
            {
              t->next[c] = make_shared();
            }
            t = t->next[c].get();
          }
          t->accept.push_back(i);
          i += 1;
        }
      }
      var que: dynamic;
      {
        var c = 0;
        while ((c < alphabets))
        {
          if (root->next[c])
          {
            root->next[c]->fail = root;
            que.push(root->next[c]);
          }
          c += 1;
        }
      }
      while ((!que.empty()))
      {
        var t = que.front();
        que.pop();
        {
          var c = 0;
          while ((c < alphabets))
          {
            if (t->next[c])
            {
              que.push(t->next[c]);
              var r = t->fail.lock();
              while (((!r->next[c]) && (r != root)))
              {
                r = r->fail.lock();
              }
              var nxt = r->next[c];
              if ((!nxt))
              {
                nxt = root;
              }
              t->next[c]->fail = nxt;
              for (var ac in nxt->accept)
              {
                t->next[c]->accept.push_back(ac);
              }
            }
            c += 1;
          }
        }
      }
    }
  func match_cpp(s: dynamic, ts: dynamic, cnt: dynamic)
  {
      var dp = cpp_construct((s.size() + 1));
      dp[0] = 1;
      var now = root.get();
      {
        var i = 0;
        while ((i < cpp_cast(s.size())))
        {
          var c = (s[i] - alphabet_base);
          while (((!now->next[c]) && (now != root.get())))
          {
            now = now->fail.lock().get();
          }
          now = now->next[c].get();
          if ((now == null))
          {
            now = root.get();
          }
          for (var k in now->accept)
          {
            (cpp_assign(dp[(i + 1)], "+=", dp[((i - ts[k].size()) + 1)])) %= M;
          }
          i += 1;
        }
      }
      cnt = dp[s.size()];
      return res;
    }
  var alphabets: dynamic;
  var alphabet_base: dynamic;
  var K: dynamic;
  var root: dynamic;
}

func main()
{
  var N: dynamic;
  read(N);
  var t: dynamic;
  {
    var i = 0;
    while ((i < N))
    {
      read(s[i]);
      i += 1;
    }
  }
  read(t);
  var res = 0;
  var match_pos = aho.match_cpp(t, s, res);
  write(res, "\n");
}
