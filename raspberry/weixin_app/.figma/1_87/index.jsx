import React from 'react';

import styles from './index.module.scss';

const Component = () => {
  return (
    <div className={styles.bottomNavBar}>
      <div className={styles.frame}>
        <img src="../image/mnkl1yrr-ufvix49.svg" className={styles.container} />
        <div className={styles.margin}>
          <p className={styles.text}>控制</p>
        </div>
      </div>
      <div className={styles.frame2}>
        <img src="../image/mnkl1yrr-quw4yh6.svg" className={styles.container2} />
        <div className={styles.margin2}>
          <p className={styles.text2}>健康</p>
        </div>
      </div>
    </div>
  );
}

export default Component;
